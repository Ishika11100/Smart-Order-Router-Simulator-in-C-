#ifndef PERFORMANCEANALYZER_H
#define PERFORMANCEANALYZER_H

#include <ostream>
#include "Portfolio.h"

// PerformanceAnalyzer: computes and reports execution quality metrics for
// a completed portfolio.  Metrics reported:
//   - Average execution price
//   - Total transaction fees
//   - Total slippage cost
//   - Fill rate  (simplified: 100% since we do not model partial fills)
//   - Implementation shortfall  (total friction cost above market reference)
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

    void printReport()                       const;
    void writeReport(std::ostream& out)      const;
};

#endif