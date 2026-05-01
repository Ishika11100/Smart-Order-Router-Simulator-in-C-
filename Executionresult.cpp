#include "ExecutionResult.h"
#include <iostream>
#include <iomanip>

// sothis is  the empty result we use as a starting point
// in the router loop. the trick is setting totalCostPerShare to the biggest double ever
// number possible so that literally ANY real venue will be cheaper than this.
// that way we dont need a special case for first venue, it just works.
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

//gets called when we actually have a venue result
// just stores all 8 fields
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

// getters data is private so these are the only way to read it from outside
// const used ot avoid functions accidentally change anything
std::string ExecutionResult::getVenueName()              const { return venueName;   }
double      ExecutionResult::getExecutionPrice()         const { return executionPrice;         }
double      ExecutionResult::getHalfSpreadCostPerShare() const { return halfSpreadCostPerShare; }
double      ExecutionResult::getSlippagePerShare()       const { return slippagePerShare;    }
double      ExecutionResult::getExchangeFeePerShare()    const { return exchangeFeePerShare;  }
double      ExecutionResult::getRegulatoryFeePerShare()  const { return regulatoryFeePerShare;  }
double      ExecutionResult::getFeePerShare()            const { return exchangeFeePerShare;  } // alias -- old code used this name
double      ExecutionResult::getTotalCostPerShare()      const { return totalCostPerShare;   }
double      ExecutionResult::getTotalCostForOrder()      const { return totalCostForOrder;  }
int         ExecutionResult::getQuantityFilled()         const { return quantityFilled;     }

// if venueName is empty it means this is the dummy sentinel, not a real result
bool ExecutionResult::isValid() const { return !venueName.empty(); }

void ExecutionResult::printSummary(const Order& order) const {
    // bail early if somehow called on the dummy, shouldnt happen but just in case
    if (!isValid()) { std::cout << "  no venue selected\n"; return; }

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "  venue=" << venueName
              << "  exec=$" << executionPrice
              << "  spread=$" << halfSpreadCostPerShare
              << "  slip=$" << slippagePerShare
              << "  fee=$" << exchangeFeePerShare
              << "  reg=$" << regulatoryFeePerShare
              << "  total=$" << totalCostForOrder << "\n";

    // order is passed in but we dont actually use it here
    // its used in writeToStream below cause compiler is sending warnings if we dont do this
    (void)order;
}

// same as printSummary but writes to a file instead of the screen
// ostream& works for both
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