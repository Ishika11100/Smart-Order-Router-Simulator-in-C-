#ifndef SMARTORDERROUTER_H
#define SMARTORDERROUTER_H

#include <vector>
#include <iostream>
#include "Venue.h"
#include "Order.h"
#include "ExecutionResult.h"
#include "MarketData.h"

// this is the brain of the whole project.
// it holds all 4 venues and for each order it:
//   1. evaluates the full cost at every venue (spread + slippage + fees + regulatory)
//   2. returns the cheapest one (routeOrder)
//   3. also runs the always-NYSE baseline separately so we can compare (routeOrderBaseline)
//
// MarketData gets passed in per-order (not stored in the router) because
// vol and ADV update throughout the trading day in real life

class SmartOrderRouter {
private:
    std::vector<Venue> venues;

    // internal helper -- builds a full ExecutionResult for one venue/order pair
    // called once per venue inside routeOrder's loop
    ExecutionResult evaluateVenue(const Venue& v,
                                  const Order& order,
                                  const MarketData& md) const;

public:
    void addVenue(const Venue& venue);
    const std::vector<Venue>& getVenues() const;

    // prints a side-by-side cost table for all venues (nice for seeing why IEX wins)
    void printVenueComparison(const Order& order, const MarketData& md) const;

    // smart strategy: loops all venues, returns the one with lowest total cost/share
    ExecutionResult routeOrder(const Order& order, const MarketData& md) const;

    // dumb baseline: always sends to venues[0] which is NYSE -- used for comparison
    ExecutionResult routeOrderBaseline(const Order& order, const MarketData& md) const;
};

#endif