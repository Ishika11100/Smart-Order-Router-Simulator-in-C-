#ifndef ORDER_H
#define ORDER_H

#include <string>
#include <iostream>
#include <iomanip>

// Abstract base class for all order types.
// Demonstrates: encapsulation, inheritance, polymorphism (pure virtual).
class Order {
protected:
    int orderId;
    std::string symbol;
    std::string side;       // "BUY" or "SELL"
    int quantity;
    double marketPrice;

public:
    Order(int id, const std::string& sym, const std::string& s, int qty, double price);
    virtual ~Order() = default;

    // Accessors
    int getOrderId() const;
    std::string getSymbol() const;
    std::string getSide() const;
    int getQuantity() const;
    double getMarketPrice() const;

    // Pure virtual: forces derived classes to identify their type
    virtual std::string getOrderType() const = 0;

    // Virtual: derived classes may override for type-specific display
    virtual void describe() const;
};

#endif