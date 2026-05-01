#include "MarketOrder.h"
#include <iostream>

// just passing everything up to the Order constructor, nothing extra to set here
MarketOrder::MarketOrder(int id, const std::string& sym, const std::string& s, int qty, double price)
    : Order(id, sym, s, qty, price) {}

std::string MarketOrder::getOrderType() const {
    return "MARKET";
}

void MarketOrder::describe() const {
    // call the parent version first to print the standard order info,
    // then tack on the market order specific note below it
    Order::describe();
    std::cout << "  -> Executes immediately at best available price.\n";
}