#include "ExecutionResult.h"
#include <iostream>
#include <iomanip>

ExecutionResult::ExecutionResult()
    : venueName(""),
      executionPrice(0.0),
      halfSpreadCostPerShare(0.0),
      slippagePerShare(0.0),
      exchangeFeePerShare(0.0),
      regulatoryFeePerShare(0.0),
      totalCostPerShare(std::numeric_limits<double>::max()),
      totalCostForOrder(0.0),
      quantityFilled(0) {}

ExecutionResult::ExecutionResult(const std::string& venue, double execPrice,
    double halfSpread, double slippage,
    double exchangeFee, double regulatoryFee,
    double totalPerShare, double totalOrderCost, int qtyFilled)
    : venueName(venue),
      executionPrice(execPrice),
      halfSpreadCostPerShare(halfSpread),
      slippagePerShare(slippage),
      exchangeFeePerShare(exchangeFee),
      regulatoryFeePerShare(regulatoryFee),
      totalCostPerShare(totalPerShare),
      totalCostForOrder(totalOrderCost),
      quantityFilled(qtyFilled) {}

std::string ExecutionResult::getVenueName()              const { return venueName;              }
double      ExecutionResult::getExecutionPrice()         const { return executionPrice;         }
double      ExecutionResult::getHalfSpreadCostPerShare() const { return halfSpreadCostPerShare; }
double      ExecutionResult::getSlippagePerShare()       const { return slippagePerShare;       }
double      ExecutionResult::getExchangeFeePerShare()    const { return exchangeFeePerShare;    }
double      ExecutionResult::getRegulatoryFeePerShare()  const { return regulatoryFeePerShare;  }
double      ExecutionResult::getFeePerShare()            const { return exchangeFeePerShare;    }
double      ExecutionResult::getTotalCostPerShare()      const { return totalCostPerShare;      }
double      ExecutionResult::getTotalCostForOrder()      const { return totalCostForOrder;      }
int         ExecutionResult::getQuantityFilled()         const { return quantityFilled;         }
bool        ExecutionResult::isValid()                   const { return !venueName.empty();     }

void ExecutionResult::printSummary(const Order& order) const {
    if (!isValid()) { std::cout << "  no venue selected\n"; return; }
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "  venue=" << venueName
              << "  exec=$" << executionPrice
              << "  spread=$" << halfSpreadCostPerShare
              << "  slip=$" << slippagePerShare
              << "  fee=$" << exchangeFeePerShare
              << "  reg=$" << regulatoryFeePerShare
              << "  total=$" << totalCostForOrder << "\n";
    (void)order;
}

void ExecutionResult::writeToStream(std::ostream& out, const Order& order) const {
    out << std::fixed << std::setprecision(4);
    out << "order " << order.getOrderId()
        << " " << order.getSymbol()
        << " " << order.getSide()
        << " qty=" << order.getQuantity()
        << " ref=$" << order.getMarketPrice() << "\n";
    out << "  venue=" << venueName
        << " exec=$" << executionPrice
        << " spread=$" << halfSpreadCostPerShare
        << " slip=$" << slippagePerShare
        << " exchfee=$" << exchangeFeePerShare
        << " regfee=$" << regulatoryFeePerShare
        << " total=$" << totalCostForOrder << "\n\n";
}