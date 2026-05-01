#include "SmartOrderRouter.h"
#include "RegulatoryFees.h"
#include <iomanip>
#include <stdexcept>

// builds a complete ExecutionResult for one (venue, order, market data) combo.
// this is where all 4 cost components actually get computed and packaged together.
ExecutionResult SmartOrderRouter::evaluateVenue(const Venue& v,
                                                 const Order& order,
                                                 const MarketData& md) const {
    const int    qty   = order.getQuantity();
    const double price = order.getMarketPrice();
    const std::string& side = order.getSide();

    // exchange costs
    double halfSpread  = v.calculateHalfSpreadCost();
    double slippage    = v.calculateSlippage(price, qty, md.dailyVolatility, md.adv);
    double exchangeFee = v.getTakerFee(); // market orders are always takers

    // actual fill price (above or below mid depending on buy/sell + slippage)
    double execPrice   = v.calculateExecutionPrice(price, qty, side,
                                                    md.dailyVolatility, md.adv);

    // regulatory fees -- computed after execPrice since SEC31 depends on it
    double regulatoryFee = RegulatoryFees::totalPerShare(execPrice, qty, side);

    double totalPerSh = halfSpread + slippage + exchangeFee + regulatoryFee;
    double totalCost  = totalPerSh * static_cast<double>(qty);

    return ExecutionResult(v.getName(), execPrice,
                           halfSpread, slippage,
                           exchangeFee, regulatoryFee,
                           totalPerSh, totalCost, qty);
}

void SmartOrderRouter::addVenue(const Venue& venue) {
    venues.push_back(venue);
}

const std::vector<Venue>& SmartOrderRouter::getVenues() const {
    return venues;
}

void SmartOrderRouter::printVenueComparison(const Order& order,
                                             const MarketData& md) const {
    if (venues.empty()) { std::cout << "  no venues registered\n"; return; }

    std::cout << std::fixed << std::setprecision(6);
    const int W = 96;
    std::cout << "\n  Venue Comparison  [Order #" << order.getOrderId()
              << "  " << order.getSymbol()
              << "  " << order.getSide()
              << "  x" << order.getQuantity() << "]\n";
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
        throw std::runtime_error("no venues registered");

    // start with the dummy sentinel (totalCostPerShare = MAX_DOUBLE).
    // any real venue will be cheaper so it replaces it on the first iteration.
    // avoids needing a special case for i == 0.
    ExecutionResult best;
    for (const auto& v : venues) {
        ExecutionResult r = evaluateVenue(v, order, md);
        if (r.getTotalCostPerShare() < best.getTotalCostPerShare())
            best = r;
    }
    return best;
}

// baseline always sends to venues[0] which is NYSE.
// no evaluation, no loop -- just picks the first one registered.
// used as the dumb comparison so we can show how much the smart routing saves.
ExecutionResult SmartOrderRouter::routeOrderBaseline(const Order& order,
                                                      const MarketData& md) const {
    if (venues.empty())
        throw std::runtime_error("no venues registered");
    return evaluateVenue(venues[0], order, md);
}