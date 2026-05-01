#ifndef PERFORMANCEANALYZER_H
#define PERFORMANCEANALYZER_H

#include <ostream>
#include "Portfolio.h"

// reads a completed portfolio and computes the TCA (transaction cost analysis) metrics.
// we pass the portfolio in by const reference -- the analyzer just reads it,
// doesnt own it or copy it. thats why explicit is on the constructor (prevents
// the compiler from accidentally converting a Portfolio into an Analyzer).
//
// metrics we compute:
//   average execution price   -- how close did we get to the market price
//   total transaction fees    -- exchange fees only
//   total slippage cost       -- market impact across all orders
//   fill rate                 -- 100% here since we dont model partial fills
//   implementation shortfall  -- total friction cost above reference price

class PerformanceAnalyzer {
private:
    const Portfolio& portfolio;

public:
    explicit PerformanceAnalyzer(const Portfolio& p);

    double getAverageExecutionPrice()   const;
    double getTotalFees()               const;
    double getTotalSlippage()           const;
    double getFillRate()                const;
    double getImplementationShortfall() const;

    void printReport()              const; // prints to terminal
    void writeReport(std::ostream& out) const; // writes to file
};

#endif