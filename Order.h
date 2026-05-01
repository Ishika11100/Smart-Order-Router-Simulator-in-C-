#ifndef ORDER_H
#define ORDER_H

#include <string>
#include <iostream>
#include <iomanip>

// base class for all order types. every order needs these 5 things:
// an id, a stock symbol, buy or sell, how many shares, and a price.
// we made this abstract (pure virtual getOrderType) so you literally
// cant create a plain "Order" -- you have to use MarketOrder or whatever
// specific type. makes sense since a generic order with no type is useless

class Order {
protected:
    // protected so MarketOrder can still see these without us making them public
    int orderId;
    std::string symbol;
    std::string side;       // "BUY" or "SELL"
    int quantity;
    double marketPrice;

public:
    Order(int id, const std::string& sym, const std::string& s, int qty, double price);
    virtual ~Order() = default; // need this virtual or deleting through a base pointer leaks memory

    // read-only access to the private fields
    int getOrderId() const;
    std::string getSymbol() const;
    std::string getSide() const;
    int getQuantity() const;
    double getMarketPrice() const;

    // pure virtual -- every subclass MUST implement this or wont compile
    virtual std::string getOrderType() const = 0;

    // subclasses can override this to add extra info when printing
    virtual void describe() const;
};

#endif