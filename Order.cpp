#include "Order.h"

Order::Order(int id, const std::string& sym, const std::string& s, int qty, double price)
    : orderId(id), symbol(sym), side(s), quantity(qty), marketPrice(price) {}

int Order::getOrderId()       const { return orderId;     }
std::string Order::getSymbol() const { return symbol;      }
std::string Order::getSide()   const { return side;        }
int Order::getQuantity()       const { return quantity;    }
double Order::getMarketPrice() const { return marketPrice; }

void Order::describe() const {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "[Order #" << orderId << "] " << getOrderType()
              << " | Symbol: " << symbol
              << " | Side: "   << side
              << " | Qty: "    << quantity
              << " | Ref Price: $" << marketPrice << "\n";
}