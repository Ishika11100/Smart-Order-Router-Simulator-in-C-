#ifndef EXECUTIONRESULT_H
#define EXECUTIONRESULT_H

#include <string>
#include <limits>
#include <ostream>
#include "Order.h"

// ExecutionResult: complete cost breakdown for routing one order to one venue.
//
// Cost components (displayed separately in reports):
//
//   halfSpreadCostPerShare   – cost of crossing the bid-ask spread
//   slippagePerShare         – price impact (Almgren-Chriss sqrt model)
//   exchangeFeePerShare      – venue's taker fee (or rebate if negative)
//   regulatoryFeePerShare    – mandatory SEC/FINRA/DTCC charges   ← NEW
//   ─────────────────────────────────────────────────────────────────
//   totalCostPerShare        – sum of all four components above
//   totalCostForOrder        – totalCostPerShare × quantityFilled

class ExecutionResult {
private:
    std::string venueName;
    double executionPrice;
    double halfSpreadCostPerShare;
    double slippagePerShare;
    double exchangeFeePerShare;      // renamed from feePerShare for clarity
    double regulatoryFeePerShare;    // SEC31 + FINRA TAF + DTCC
    double totalCostPerShare;
    double totalCostForOrder;
    int    quantityFilled;

public:
    // Default constructor: produces an "invalid" sentinel with max cost.
    // Used by routeOrder() as the initial "best" before any venue is evaluated.
    ExecutionResult();

    ExecutionResult(const std::string& venue,
                    double execPrice,
                    double halfSpread,
                    double slippage,
                    double exchangeFee,
                    double regulatoryFee,
                    double totalPerShare,
                    double totalOrderCost,
                    int    qtyFilled);

    std::string getVenueName()              const;
    double      getExecutionPrice()         const;
    double      getHalfSpreadCostPerShare() const;
    double      getSlippagePerShare()       const;
    double      getExchangeFeePerShare()    const;
    double      getRegulatoryFeePerShare()  const;
    double      getFeePerShare()            const;  // returns exchangeFee (backward compat)
    double      getTotalCostPerShare()      const;
    double      getTotalCostForOrder()      const;
    int         getQuantityFilled()         const;
    bool        isValid()                   const;

    void printSummary(const Order& order)                      const;
    void writeToStream(std::ostream& out, const Order& order)  const;
};

#endif