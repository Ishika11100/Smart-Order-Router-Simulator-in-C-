#include "SmartOrderRouter.h"
#include <iomanip>
#include <stdexcept>

// ── Private helper ────────────────────────────────────────────────────────────

ExecutionResult SmartOrderRouter::evaluateVenue(const Venue& v, const Order& order) const {
    double halfSpread = v.calculateHalfSpreadCost();
    double slippage   = v.calculateSlippage(order.getQuantity());
    double fee        = v.getTransactionFee();
    double execPrice  = v.calculateExecutionPrice(order.getMarketPrice(),
                                                   order.getQuantity(),
                                                   order.getSide());
    double totalPerSh = v.calculateTotalCostPerShare(order.getQuantity());
    double totalCost  = totalPerSh * static_cast<double>(order.getQuantity());

    return ExecutionResult(v.getName(), execPrice, halfSpread, slippage,
                           fee, totalPerSh, totalCost, order.getQuantity());
}

// ── Public interface ──────────────────────────────────────────────────────────

void SmartOrderRouter::addVenue(const Venue& venue) {
    venues.push_back(venue);
}

const std::vector<Venue>& SmartOrderRouter::getVenues() const {
    return venues;
}

void SmartOrderRouter::printVenueComparison(const Order& order) const {
    if (venues.empty()) { std::cout << "  No venues registered.\n"; return; }

    std::cout << std::fixed << std::setprecision(4);
    const int W = 82;
    std::cout << "\n  Venue Comparison  [Order #" << order.getOrderId()
              << "  " << order.getSymbol() << "  " << order.getSide()
              << "  x" << order.getQuantity() << "]\n";
    std::cout << "  " << std::string(W, '-') << "\n";
    std::cout << "  " << std::left
              << std::setw(10) << "Venue"
              << std::setw(14) << "Exec Price"
              << std::setw(14) << "Half-Spread"
              << std::setw(14) << "Slippage"
              << std::setw(12) << "Fee/sh"
              << std::setw(14) << "Total/sh" << "\n";
    std::cout << "  " << std::string(W, '-') << "\n";

    for (const auto& v : venues) {
        ExecutionResult r = evaluateVenue(v, order);
        std::cout << "  " << std::left
                  << std::setw(10) << v.getName()
                  << std::setw(14) << r.getExecutionPrice()
                  << std::setw(14) << r.getHalfSpreadCostPerShare()
                  << std::setw(14) << r.getSlippagePerShare()
                  << std::setw(12) << r.getFeePerShare()
                  << std::setw(14) << r.getTotalCostPerShare() << "\n";
    }
    std::cout << "  " << std::string(W, '-') << "\n";
}

ExecutionResult SmartOrderRouter::routeOrder(const Order& order) const {
    if (venues.empty())
        throw std::runtime_error("SmartOrderRouter: no venues registered.");

    ExecutionResult best;   // initialised with max-cost sentinel
    for (const auto& v : venues) {
        ExecutionResult r = evaluateVenue(v, order);
        if (r.getTotalCostPerShare() < best.getTotalCostPerShare())
            best = r;
    }
    return best;
}

ExecutionResult SmartOrderRouter::routeOrderBaseline(const Order& order) const {
    if (venues.empty())
        throw std::runtime_error("SmartOrderRouter: no venues registered.");
    // Baseline: always send to first venue (e.g., NYSE)
    return evaluateVenue(venues[0], order);
}