#ifndef ORDERSPLITTER_H
#define ORDERSPLITTER_H

#include "Order.h"
#include "Venue.h"
#include "MarketData.h"
#include "ExecutionResult.h"
#include <vector>
#include <string>

// one slice of a split order -- how much goes to which venue
struct VenueSlice {
    std::string venueName;
    int         quantity;
    double      participationRate; // qty / ADV for this slice
};

// the full split plan for one large order
struct SplitPlan {
    std::vector<VenueSlice> slices;
    double totalQty          = 0;
    double estimatedSavings  = 0.0; // vs sending everything to one venue
    bool   wasSplit          = false;
};

// checks if an order needs splitting and if so figures out how to allocate it.
//
// when to split:
//   participation_rate = qty / ADV
//   if that's above 1%, we split. below 1%, single venue is fine.
//   the 1% threshold comes from Almgren & Chriss (2000) -- below that level
//   the savings from splitting dont justify the added complexity.
//
// how we allocate:
//   inverse-cost weighting: weight_i = 1 / totalCostPerShare_i
//   cheaper venue gets a bigger slice.
//   each venue is capped at MAX_VENUE_PARTICIPATION * ADV so no single
//   venue gets hammered. overflow redistributes to cheapest with remaining room.

class OrderSplitter {
public:
    static constexpr double SPLIT_THRESHOLD        = 0.01;  // 1% of ADV
    static constexpr double MAX_VENUE_PARTICIPATION = 0.008; // 0.8% per venue cap

    static bool   needsSplitting   (const Order& order, const MarketData& md);
    static double participationRate(const Order& order, const MarketData& md);

    static SplitPlan split(const Order& order,
                           const std::vector<Venue>& venues,
                           const MarketData& md);

    static void printPlan(const SplitPlan& plan,
                          const Order& order,
                          const MarketData& md);
};

#endif