#ifndef PORTFOLIO_H
#define PORTFOLIO_H

#include <vector>
#include <memory>
#include <string>
#include "Order.h"
#include "ExecutionResult.h"

// bundles an order and its execution result together so they travel as one unit.
// we use shared_ptr for the order because the same order object lives in both
// smartPortfolio and baselinePortfolio -- no copying, just two references.
struct RoutedOrder {
    std::shared_ptr<Order> order;
    ExecutionResult result;
};

// Portfolio is basically a folder.
// every time an order gets routed, the result goes in here.
// two portfolios run in parallel -- one for smart routing, one for baseline.
// PerformanceAnalyzer reads from these at the end to compute the summary stats.
class Portfolio {
private:
    std::vector<RoutedOrder> history;
    std::string strategyName;

public:
    explicit Portfolio(const std::string& name = "Strategy");

    void addRoutedOrder(std::shared_ptr<Order> order, const ExecutionResult& result);

    const std::vector<RoutedOrder>& getHistory()      const;
    const std::string&              getStrategyName() const;
    size_t                          size()            const;

    double getTotalCost()           const;
    double getTotalQuantityFilled() const;
};

#endif