#ifndef EXECUTIONRESULT_H
#define EXECUTIONRESULT_H

#include <string>
#include <limits>
#include <ostream>
#include "Order.h"

// ExecutionResult: stores and reports the outcome of routing a single order
// to a specific venue.
class ExecutionResult {
private:
    std::string venueName;
    double executionPrice;
    double halfSpreadCostPerShare;
    double slippagePerShare;
    double feePerShare;
    double totalCostPerShare;
    double totalCostForOrder;
    int    quantityFilled;

public:
    // Default constructor produces an "invalid" sentinel with max cost
    ExecutionResult();

    ExecutionResult(const std::string& venue,
                    double execPrice,
                    double halfSpread,
                    double slippage,
                    double fee,
                    double totalPerShare,
                    double totalOrderCost,
                    int    qtyFilled);

    std::string getVenueName()              const;
    double      getExecutionPrice()         const;
    double      getHalfSpreadCostPerShare() const;
    double      getSlippagePerShare()       const;
    double      getFeePerShare()            const;
    double      getTotalCostPerShare()      const;
    double      getTotalCostForOrder()      const;
    int         getQuantityFilled()         const;
    bool        isValid()                   const;

    void printSummary(const Order& order)              const;
    void writeToStream(std::ostream& out, const Order& order) const;
};

#endif