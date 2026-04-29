#include "ExecutionResult.h"
#include <iostream>
#include <iomanip>

ExecutionResult::ExecutionResult()
    : venueName(""),
      executionPrice(0.0),
      halfSpreadCostPerShare(0.0),
      slippagePerShare(0.0),
      feePerShare(0.0),
      totalCostPerShare(std::numeric_limits<double>::max()),
      totalCostForOrder(0.0),
      quantityFilled(0) {}

ExecutionResult::ExecutionResult(const std::string& venue, double execPrice,
    double halfSpread, double slippage, double fee,
    double totalPerShare, double totalOrderCost, int qtyFilled)
    : venueName(venue),
      executionPrice(execPrice),
      halfSpreadCostPerShare(halfSpread),
      slippagePerShare(slippage),
      feePerShare(fee),
      totalCostPerShare(totalPerShare),
      totalCostForOrder(totalOrderCost),
      quantityFilled(qtyFilled) {}

std::string ExecutionResult::getVenueName()              const { return venueName;              }
double      ExecutionResult::getExecutionPrice()         const { return executionPrice;         }
double      ExecutionResult::getHalfSpreadCostPerShare() const { return halfSpreadCostPerShare; }
double      ExecutionResult::getSlippagePerShare()       const { return slippagePerShare;       }
double      ExecutionResult::getFeePerShare()            const { return feePerShare;            }
double      ExecutionResult::getTotalCostPerShare()      const { return totalCostPerShare;      }
double      ExecutionResult::getTotalCostForOrder()      const { return totalCostForOrder;      }
int         ExecutionResult::getQuantityFilled()         const { return quantityFilled;         }
bool        ExecutionResult::isValid()                   const { return !venueName.empty();     }

void ExecutionResult::printSummary(const Order& order) const {
    if (!isValid()) { std::cout << "  No venue selected.\n"; return; }
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "  Venue          : " << venueName                   << "\n";
    std::cout << "  Exec Price     : $" << executionPrice             << "\n";
    std::cout << "  Half-Spread/sh : $" << halfSpreadCostPerShare     << "\n";
    std::cout << "  Slippage/sh    : $" << slippagePerShare           << "\n";
    std::cout << "  Fee/sh         : $" << feePerShare                << "\n";
    std::cout << "  Total Cost/sh  : $" << totalCostPerShare          << "\n";
    std::cout << "  Total Cost     : $" << totalCostForOrder          << "\n";
    std::cout << "  Qty Filled     :  " << quantityFilled             << "\n";
    (void)order; // order context is printed by describe() in main
}

void ExecutionResult::writeToStream(std::ostream& out, const Order& order) const {
    out << std::fixed << std::setprecision(4);
    out << "Order #" << order.getOrderId()
        << " | " << order.getOrderType()
        << " | " << order.getSymbol()
        << " | " << order.getSide()
        << " | Qty: " << order.getQuantity()
        << " | Ref: $" << order.getMarketPrice() << "\n";
    out << "  Venue: "      << venueName
        << " | Exec: $"    << executionPrice
        << " | Cost/sh: $" << totalCostPerShare
        << " | Total: $"   << totalCostForOrder << "\n\n";
}