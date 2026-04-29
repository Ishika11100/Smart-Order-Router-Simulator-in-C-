#ifndef PORTFOLIO_H
#define PORTFOLIO_H

#include <vector>
#include <memory>
#include <string>
#include "Order.h"
#include "ExecutionResult.h"

// Bundles an order pointer with its execution outcome for post-trade analysis.
struct RoutedOrder {
    std::shared_ptr<Order> order;
    ExecutionResult result;
};

// Portfolio: accumulates routed orders for a single strategy and provides
// aggregate cost figures to the PerformanceAnalyzer.
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

    double getTotalCost()                             const;
    double getTotalQuantityFilled()                   const;
};

#endif