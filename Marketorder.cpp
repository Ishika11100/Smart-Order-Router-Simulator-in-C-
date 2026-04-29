#include "MarketOrder.h"
#include <iostream>
#include <iomanip>

MarketOrder::MarketOrder(int id, const std::string& sym, const std::string& s, int qty, double price)
    : Order(id, sym, s, qty, price) {}

std::string MarketOrder::getOrderType() const {
    return "MARKET";
}

void MarketOrder::describe() const {
    // Calls base describe() and appends market-order-specific note
    Order::describe();
    std::cout << "  -> Executes immediately at best available price.\n";
}