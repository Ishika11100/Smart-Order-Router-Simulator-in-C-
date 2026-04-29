#ifndef SMARTORDERROUTER_H
#define SMARTORDERROUTER_H

#include <vector>
#include <iostream>
#include "Venue.h"
#include "Order.h"
#include "ExecutionResult.h"

// SmartOrderRouter: evaluates all venues for a given order and routes to
// the one with the lowest total execution cost.
// Also exposes a baseline strategy (always route to the first venue) for
// performance comparison.
class SmartOrderRouter {
private:
    std::vector<Venue> venues;

    // Internal helper: builds an ExecutionResult for one venue/order pair
    ExecutionResult evaluateVenue(const Venue& v, const Order& order) const;

public:
    void addVenue(const Venue& venue);
    const std::vector<Venue>& getVenues() const;

    // Print a side-by-side cost comparison table for all venues
    void printVenueComparison(const Order& order) const;

    // Smart strategy: picks the venue with the minimum total cost per share
    ExecutionResult routeOrder(const Order& order) const;

    // Baseline strategy: always routes to the first registered venue
    ExecutionResult routeOrderBaseline(const Order& order) const;
};

#endif