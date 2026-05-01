#ifndef MARKETORDER_H
#define MARKETORDER_H

#include "Order.h"

// MarketOrder is the only order type we actually use in this project.
// a market order just means "execute right now at whatever price the
// market gives you" -- no conditions, no waiting.
//
// it inherits everything from Order (the 5 fields, all the getters)
// and just adds its own getOrderType() and a slightly extended describe()

class MarketOrder : public Order {
public:
    MarketOrder(int id, const std::string& sym, const std::string& s, int qty, double price);

    std::string getOrderType() const override; // returns "MARKET"
    void describe() const override;            // calls parent describe() then adds one line
};

#endif