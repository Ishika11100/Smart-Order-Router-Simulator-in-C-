#ifndef ORDERSPLITTER_H
#define ORDERSPLITTER_H

#include "Order.h"
#include "Venue.h"
#include "MarketData.h"
#include "ExecutionResult.h"
#include <vector>
#include <string>

// ── VenueSlice ────────────────────────────────────────────────────────────────
// One piece of a split order — sent to a specific venue.
struct VenueSlice {
    std::string venueName;
    int         quantity;          // shares allocated to this venue
    double      participationRate; // qty / ADV at this venue (impact indicator)
};

// ── SplitPlan ─────────────────────────────────────────────────────────────────
// The full result of splitting one large order across venues.
struct SplitPlan {
    std::vector<VenueSlice> slices;      // one per venue that receives shares
    double totalQty         = 0;
    double estimatedSavings = 0.0;       // vs. sending everything to one venue
    bool   wasSplit         = false;     // false if order was small enough to not split
};

// ── OrderSplitter ─────────────────────────────────────────────────────────────
//
// Answers two questions:
//   1. Is this order large enough to need splitting?
//   2. If yes, how should it be allocated across venues?
//
// ── Threshold formula ─────────────────────────────────────────────────────────
//
// An order is "large" if its participation rate exceeds SPLIT_THRESHOLD.
//
//   participation_rate = qty / ADV
//
// SPLIT_THRESHOLD = 0.01 (1% of ADV) is the standard institutional boundary.
// Below 1%: send the whole order to the best single venue — impact negligible.
// Above 1%: split across venues to keep each venue's slice below the threshold.
//
// Why 1%?
//   Academic literature (Almgren & Chriss 2000, Kissell 2006) and industry
//   practice both treat ~1% participation as the point where market impact
//   becomes material enough to justify more complex execution. Below 1% the
//   transaction cost savings from splitting are smaller than the operational
//   overhead of managing multiple child orders.
//
// ── Split allocation method ───────────────────────────────────────────────────
//
// We use COST-WEIGHTED allocation:
//   For each venue i, compute the marginal cost of adding one more share
//   at a neutral (equal) split. Venues with LOWER cost get a LARGER slice.
//
//   Specifically:
//     weight_i = 1 / totalCostPerShare_i(qty/N)   where N = number of venues
//     allocation_i = qty × (weight_i / sum_weights)
//
// Then we cap each venue at SPLIT_THRESHOLD × ADV shares to prevent any
// single slice from exceeding the impact limit.
//
// The uncapped remainder is re-allocated to venues that still have capacity,
// again proportionally by inverse cost.
//
// Why not TWAP/VWAP?
//   TWAP (Time Weighted Average Price) and VWAP (Volume Weighted Average Price)
//   are TIME-based algorithms — they spread execution across minutes or hours.
//   This simulator executes orders instantaneously (no time dimension), so we
//   do the cross-VENUE version of the same idea: spread the order across
//   exchanges rather than across time.

class OrderSplitter {
public:
    // 1% of ADV = threshold for needing a split
    static constexpr double SPLIT_THRESHOLD = 0.01;

    // Safety cap: never allocate more than this % of ADV to any single venue
    static constexpr double MAX_VENUE_PARTICIPATION = 0.008;  // 0.8% per venue

    // Returns true if this order's participation rate exceeds the threshold
    static bool needsSplitting(const Order& order, const MarketData& md);

    // Returns the participation rate (qty / ADV) for this order
    static double participationRate(const Order& order, const MarketData& md);

    // Builds a split plan allocating the order across venues.
    // If the order doesn't need splitting, returns a single-venue plan for
    // the cheapest venue (same as routeOrder).
    static SplitPlan split(const Order& order,
                           const std::vector<Venue>& venues,
                           const MarketData& md);

    // Prints a human-readable split plan to stdout
    static void printPlan(const SplitPlan& plan, const Order& order,
                          const MarketData& md);
};

#endif