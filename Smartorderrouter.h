#ifndef SMARTORDERROUTER_H
#define SMARTORDERROUTER_H

#include <vector>
#include <iostream>
#include "Venue.h"
#include "Order.h"
#include "ExecutionResult.h"
#include "MarketData.h"

// SmartOrderRouter: evaluates all registered venues for a given order and
// routes to the one with the lowest TOTAL cost per share.
//
// "Total cost" now includes ALL four components:
//   1. Half bid-ask spread  (venue-specific)
//   2. Market impact        (Almgren-Chriss sqrt, symbol + venue specific)
//   3. Exchange fee         (venue-specific, taker rate)
//   4. Regulatory fees      (mandatory, computed from exec price + side)
//
// Market data (volatility, ADV) is passed per-order, not stored in the router,
// because in production these values update continuously throughout the day.

class SmartOrderRouter {
private:
    std::vector<Venue> venues;

    // Builds a full ExecutionResult for one (venue, order, market data) triple
    ExecutionResult evaluateVenue(const Venue& v,
                                  const Order& order,
                                  const MarketData& md) const;

public:
    void addVenue(const Venue& venue);
    const std::vector<Venue>& getVenues() const;

    // Side-by-side cost breakdown table across all venues for one order
    void printVenueComparison(const Order& order, const MarketData& md) const;

    // Smart strategy: picks the venue with the minimum total cost per share
    ExecutionResult routeOrder(const Order& order, const MarketData& md) const;

    // Baseline strategy: always routes to the first registered venue
    ExecutionResult routeOrderBaseline(const Order& order, const MarketData& md) const;
};

#endif