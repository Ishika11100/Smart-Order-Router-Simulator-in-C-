#ifndef EXECUTIONRESULT_H
#define EXECUTIONRESULT_H

#include <string>
#include <limits>
#include <ostream>
#include "Order.h"

// ExecutionResult is the receipt for routing one order to one venue.
// we break the cost into 4 parts so the report shows exactly where
// the money went instead of just showing a lump total.
//
//   halfSpreadCostPerShare  -- cost of crossing the bid/ask spread
//   slippagePerShare        -- price impact (Almgren-Chriss sqrt model)
//   exchangeFeePerShare     -- the venue's taker fee per share
//   regulatoryFeePerShare   -- SEC31 + FINRA TAF + DTCC (mandatory govt fees)
//
//   totalCostPerShare = all four added together
//   totalCostForOrder = totalCostPerShare * quantity

class ExecutionResult {
private:
    std::string venueName;
    double executionPrice;
    double halfSpreadCostPerShare;
    double slippagePerShare;
    double exchangeFeePerShare;   // used to be called feePerShare, renamed to be clearer
    double regulatoryFeePerShare; // SEC31 + FINRA TAF + DTCC
    double totalCostPerShare;
    double totalCostForOrder;
    int    quantityFilled;

public:
    // default constructor sets totalCostPerShare to MAX_DOUBLE on purpose --
    // this is the "dummy" we start with in the routing loop so the first
    // real venue always replaces it. avoids needing a special case for i=0.
    ExecutionResult();

    // real constructor, called once we have actual numbers from a venue
    ExecutionResult(const std::string& venue,
                    double execPrice,
                    double halfSpread,
                    double slippage,
                    double exchangeFee,
                    double regulatoryFee,
                    double totalPerShare,
                    double totalOrderCost,
                    int    qtyFilled);

    // getters -- data is private so these are the only way to read it from outside
    std::string getVenueName()              const;
    double      getExecutionPrice()         const;
    double      getHalfSpreadCostPerShare() const;
    double      getSlippagePerShare()       const;
    double      getExchangeFeePerShare()    const;
    double      getRegulatoryFeePerShare()  const;
    double      getFeePerShare()            const; // old name kept for backward compat
    double      getTotalCostPerShare()      const;
    double      getTotalCostForOrder()      const;
    int         getQuantityFilled()         const;

    // returns false if this is the dummy sentinel (venueName is empty)
    bool        isValid()                   const;

    // prints cost breakdown to terminal
    void printSummary(const Order& order)                     const;

    // same but writes to a file -- ostream& works for both screen and file
    // thats why we dont need two separate write functions
    void writeToStream(std::ostream& out, const Order& order) const;
};

#endif