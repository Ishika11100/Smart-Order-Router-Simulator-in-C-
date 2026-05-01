#include "Order.h"

// initializer list is faster than assigning in the body, so we use that
Order::Order(int id, const std::string& sym, const std::string& s, int qty, double price)
    : orderId(id), symbol(sym), side(s), quantity(qty), marketPrice(price) {}

// just returning the stored values, nothing complicated here
int Order::getOrderId()        const { return orderId;     }
std::string Order::getSymbol() const { return symbol;      }
std::string Order::getSide()   const { return side;        }
int Order::getQuantity()       const { return quantity;    }
double Order::getMarketPrice() const { return marketPrice; }

void Order::describe() const {
    std::cout << std::fixed << std::setprecision(2);
    // calls getOrderType() which resolves to the subclass version at runtime
    // thats the polymorphism part -- same describe() but prints MARKET or LIMIT etc
    std::cout << "[Order #" << orderId << "] " << getOrderType()
              << " | Symbol: " << symbol
              << " | Side: "   << side
              << " | Qty: "    << quantity
              << " | Ref Price: $" << marketPrice << "\n";
}