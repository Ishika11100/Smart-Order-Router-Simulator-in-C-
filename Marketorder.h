#ifndef MARKETORDER_H
#define MARKETORDER_H

#include "Order.h"

// MarketOrder: executes immediately at the best available market price.
// Demonstrates: inheritance and polymorphism by overriding getOrderType().
class MarketOrder : public Order {
public:
    MarketOrder(int id, const std::string& sym, const std::string& s, int qty, double price);

    std::string getOrderType() const override;
    void describe() const override;
};

#endif