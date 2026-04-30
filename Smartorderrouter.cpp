#include "SmartOrderRouter.h"
#include "RegulatoryFees.h"
#include <iomanip>
#include <stdexcept>

// ── Private helper ────────────────────────────────────────────────────────────

ExecutionResult SmartOrderRouter::evaluateVenue(const Venue& v,
                                                 const Order& order,
                                                 const MarketData& md) const {
    const int    qty   = order.getQuantity();
    const double price = order.getMarketPrice();
    const std::string& side = order.getSide();

    // Exchange-side costs
    double halfSpread    = v.calculateHalfSpreadCost();
    double slippage      = v.calculateSlippage(price, qty, md.dailyVolatility, md.adv);
    double exchangeFee   = v.getTakerFee();   // market orders are always takers
    double execPrice     = v.calculateExecutionPrice(price, qty, side,
                                                     md.dailyVolatility, md.adv);

    // Regulatory costs (mandatory; computed on the actual execution price)
    double regulatoryFee = RegulatoryFees::totalPerShare(execPrice, qty, side);

    // Grand total and order total
    double totalPerSh  = halfSpread + slippage + exchangeFee + regulatoryFee;
    double totalCost   = totalPerSh * static_cast<double>(qty);

    return ExecutionResult(v.getName(), execPrice,
                           halfSpread, slippage,
                           exchangeFee, regulatoryFee,
                           totalPerSh, totalCost, qty);
}

// ── Public interface ──────────────────────────────────────────────────────────

void SmartOrderRouter::addVenue(const Venue& venue) {
    venues.push_back(venue);
}

const std::vector<Venue>& SmartOrderRouter::getVenues() const {
    return venues;
}

void SmartOrderRouter::printVenueComparison(const Order& order,
                                             const MarketData& md) const {
    if (venues.empty()) { std::cout << "  No venues registered.\n"; return; }

    std::cout << std::fixed << std::setprecision(6);
    const int W = 96;

    std::cout << "\n  Venue Comparison  [Order #" << order.getOrderId()
              << "  " << order.getSymbol()
              << "  σ_d=" << md.dailyVolatility
              << "  ADV=" << static_cast<long long>(md.adv)
              << "]\n";
    std::cout << "  [" << order.getSide() << "  x" << order.getQuantity()
              << "  @$" << order.getMarketPrice() << "]\n";
    std::cout << "  " << std::string(W, '-') << "\n";
    std::cout << "  " << std::left
              << std::setw(10) << "Venue"
              << std::setw(13) << "Exec Price"
              << std::setw(13) << "HalfSpread"
              << std::setw(13) << "Slippage"
              << std::setw(13) << "ExchFee"
              << std::setw(13) << "RegFee"
              << std::setw(13) << "Total/sh" << "\n";
    std::cout << "  " << std::string(W, '-') << "\n";

    for (const auto& v : venues) {
        ExecutionResult r = evaluateVenue(v, order, md);
        std::cout << "  " << std::left
                  << std::setw(10) << v.getName()
                  << std::setw(13) << r.getExecutionPrice()
                  << std::setw(13) << r.getHalfSpreadCostPerShare()
                  << std::setw(13) << r.getSlippagePerShare()
                  << std::setw(13) << r.getExchangeFeePerShare()
                  << std::setw(13) << r.getRegulatoryFeePerShare()
                  << std::setw(13) << r.getTotalCostPerShare() << "\n";
    }
    std::cout << "  " << std::string(W, '-') << "\n";
}

ExecutionResult SmartOrderRouter::routeOrder(const Order& order,
                                              const MarketData& md) const {
    if (venues.empty())
        throw std::runtime_error("SmartOrderRouter: no venues registered.");

    ExecutionResult best;   // sentinel: totalCostPerShare = numeric_limits::max
    for (const auto& v : venues) {
        ExecutionResult r = evaluateVenue(v, order, md);
        if (r.getTotalCostPerShare() < best.getTotalCostPerShare())
            best = r;
    }
    return best;
}

ExecutionResult SmartOrderRouter::routeOrderBaseline(const Order& order,
                                                      const MarketData& md) const {
    if (venues.empty())
        throw std::runtime_error("SmartOrderRouter: no venues registered.");
    return evaluateVenue(venues[0], order, md);   // always NYSE (first registered)
}