#include "OrderSplitter.h"
#include "RegulatoryFees.h"

#include <iostream>
#include <iomanip>
#include <numeric>
#include <algorithm>
#include <stdexcept>
#include <cmath>

// ── Helpers ───────────────────────────────────────────────────────────────────

// Cost per share at a venue for a given hypothetical qty, using live or
// hardcoded spread depending on what's available.
static double costPerShare(const Venue& v, double price, int qty,
                            const std::string& side, const MarketData& md) {
    // Use live NBBO spread if available, else venue's hardcoded spread.
    // In either case we still use the venue's own fee.
    double halfSpread = (md.liveSpread() > 0.0)
                        ? md.liveSpread() / 2.0
                        : v.calculateHalfSpreadCost();

    double slippage  = v.calculateSlippage(price, qty, md.dailyVolatility, md.adv);
    double exchFee   = v.getTakerFee();

    // Compute approximate exec price for regulatory fee calculation
    double execPrice = (side == "SELL")
                       ? price - halfSpread - slippage
                       : price + halfSpread + slippage;
    double regFee    = RegulatoryFees::totalPerShare(execPrice, qty, side);

    return halfSpread + slippage + exchFee + regFee;
}

// ── Public methods ────────────────────────────────────────────────────────────

double OrderSplitter::participationRate(const Order& order, const MarketData& md) {
    if (md.adv <= 0.0) return 1.0;   // treat zero-ADV as 100% participation
    return static_cast<double>(order.getQuantity()) / md.adv;
}

bool OrderSplitter::needsSplitting(const Order& order, const MarketData& md) {
    return participationRate(order, md) > SPLIT_THRESHOLD;
}

// ── Core splitting algorithm ──────────────────────────────────────────────────

SplitPlan OrderSplitter::split(const Order& order,
                                const std::vector<Venue>& venues,
                                const MarketData& md) {
    if (venues.empty())
        throw std::runtime_error("OrderSplitter: no venues registered.");

    SplitPlan plan;
    plan.totalQty = static_cast<double>(order.getQuantity());

    const int    totalQty = order.getQuantity();
    const double price    = (md.livePrice > 0.0)
                            ? md.livePrice
                            : order.getMarketPrice();
    const std::string& side = order.getSide();

    // ── Case 1: order is small — pick cheapest venue, no split needed ─────────
    if (!needsSplitting(order, md)) {
        plan.wasSplit = false;

        // Find cheapest venue
        int    bestIdx  = 0;
        double bestCost = std::numeric_limits<double>::max();
        for (size_t i = 0; i < venues.size(); ++i) {
            double c = costPerShare(venues[i], price, totalQty, side, md);
            if (c < bestCost) { bestCost = c; bestIdx = static_cast<int>(i); }
        }

        VenueSlice s;
        s.venueName         = venues[bestIdx].getName();
        s.quantity          = totalQty;
        s.participationRate = participationRate(order, md);
        plan.slices.push_back(s);
        plan.estimatedSavings = 0.0;
        return plan;
    }

    // ── Case 2: large order — split across venues ─────────────────────────────
    plan.wasSplit = true;

    // Step A: compute a neutral "trial" qty per venue (equal split) and
    //         measure cost at that level to get weights.
    int trialQtyEach = std::max(1, totalQty / static_cast<int>(venues.size()));

    std::vector<double> weights(venues.size());
    double sumWeights = 0.0;
    for (size_t i = 0; i < venues.size(); ++i) {
        double c = costPerShare(venues[i], price, trialQtyEach, side, md);
        // Inverse cost weight: cheaper venue → higher weight → more shares
        weights[i] = (c > 1e-12) ? 1.0 / c : 1e12;
        sumWeights += weights[i];
    }

    // Step B: first-pass allocation proportional to inverse cost, capped at
    //         MAX_VENUE_PARTICIPATION × ADV per venue.
    int    maxPerVenue = static_cast<int>(MAX_VENUE_PARTICIPATION * md.adv);
    maxPerVenue = std::max(maxPerVenue, 1);   // never cap below 1 share

    std::vector<int> alloc(venues.size(), 0);
    int remaining = totalQty;
    int overflow  = 0;   // shares that hit the cap and need redistribution

    for (size_t i = 0; i < venues.size(); ++i) {
        int proposed = static_cast<int>((weights[i] / sumWeights)
                                        * static_cast<double>(totalQty));
        if (proposed > maxPerVenue) {
            alloc[i] = maxPerVenue;
            overflow += proposed - maxPerVenue;
        } else {
            alloc[i] = proposed;
        }
        remaining -= alloc[i];
    }
    // `remaining` handles any integer rounding leftover
    // Add remaining + overflow back to cheapest non-capped venue

    // Step C: redistribute overflow + rounding remainder to venues
    //         that still have capacity, cheapest first.
    int leftover = remaining + overflow;
    if (leftover > 0) {
        // Sort venues by cost ascending (cheapest first)
        std::vector<size_t> order_by_cost(venues.size());
        std::iota(order_by_cost.begin(), order_by_cost.end(), 0);
        std::sort(order_by_cost.begin(), order_by_cost.end(),
                  [&](size_t a, size_t b) {
                      return costPerShare(venues[a], price, alloc[a]+1, side, md)
                           < costPerShare(venues[b], price, alloc[b]+1, side, md);
                  });

        for (size_t idx : order_by_cost) {
            if (leftover <= 0) break;
            int capacity = maxPerVenue - alloc[idx];
            if (capacity <= 0) continue;
            int add = std::min(leftover, capacity);
            alloc[idx] += add;
            leftover   -= add;
        }

        // If all venues are at cap, force remaining into cheapest venue
        if (leftover > 0) {
            size_t cheapest = order_by_cost[0];
            alloc[cheapest] += leftover;
        }
    }

    // Step D: build plan.slices (only include venues that got > 0 shares)
    double totalCostSplit   = 0.0;

    // Cost if we had just sent everything to the cheapest single venue
    double cheapestSingleCost = std::numeric_limits<double>::max();
    for (const auto& v : venues) {
        double c = costPerShare(v, price, totalQty, side, md)
                   * static_cast<double>(totalQty);
        cheapestSingleCost = std::min(cheapestSingleCost, c);
    }

    for (size_t i = 0; i < venues.size(); ++i) {
        if (alloc[i] <= 0) continue;

        VenueSlice s;
        s.venueName         = venues[i].getName();
        s.quantity          = alloc[i];
        s.participationRate = static_cast<double>(alloc[i]) / md.adv;
        plan.slices.push_back(s);

        totalCostSplit += costPerShare(venues[i], price, alloc[i], side, md)
                          * static_cast<double>(alloc[i]);
    }

    plan.estimatedSavings = cheapestSingleCost - totalCostSplit;
    return plan;
}

// ── Pretty-print ──────────────────────────────────────────────────────────────

void OrderSplitter::printPlan(const SplitPlan& plan, const Order& order,
                               const MarketData& md) {
    double pr = participationRate(order, md) * 100.0;
    double price = (md.livePrice > 0.0) ? md.livePrice : order.getMarketPrice();

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "\n  ── Order Splitting Analysis ─────────────────────────────────────\n";
    std::cout << "  Participation rate : " << pr << "% of ADV"
              << "  (threshold: " << (SPLIT_THRESHOLD * 100.0) << "%)\n";

    if (!plan.wasSplit) {
        std::cout << "  Decision           : SINGLE VENUE (order is small, no split needed)\n";
        std::cout << "  Venue              : " << plan.slices[0].venueName
                  << "  (" << plan.slices[0].quantity << " shares)\n";
        return;
    }

    std::cout << "  Decision           : SPLIT across " << plan.slices.size()
              << " venue(s)\n";
    std::cout << "  Estimated savings  : $" << plan.estimatedSavings
              << " vs sending all to cheapest single venue\n";

    // Live spread line
    if (md.liveSpread() > 0.0) {
        std::cout << "  Live NBBO spread   : $" << md.liveSpread()
                  << "  (bid $" << md.liveBid << " / ask $" << md.liveAsk << ")\n";
    }
    std::cout << "  Live price used    : $" << price << "\n";

    std::cout << "\n  " << std::left
              << std::setw(10) << "Venue"
              << std::setw(12) << "Qty"
              << std::setw(16) << "Participation"
              << std::setw(12) << "% of total" << "\n";
    std::cout << "  " << std::string(50, '-') << "\n";

    for (const auto& s : plan.slices) {
        double pct = (static_cast<double>(s.quantity) / order.getQuantity()) * 100.0;
        std::cout << "  " << std::left
                  << std::setw(10) << s.venueName
                  << std::setw(12) << s.quantity
                  << std::setw(16) << (s.participationRate * 100.0)
                  << std::setw(12) << pct << "%\n";
    }
    std::cout << "  " << std::string(50, '-') << "\n";
}