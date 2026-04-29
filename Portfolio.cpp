#include "Portfolio.h"

Portfolio::Portfolio(const std::string& name) : strategyName(name) {}

void Portfolio::addRoutedOrder(std::shared_ptr<Order> order, const ExecutionResult& result) {
    history.push_back({order, result});
}

const std::vector<RoutedOrder>& Portfolio::getHistory()      const { return history;      }
const std::string&              Portfolio::getStrategyName() const { return strategyName; }
size_t                          Portfolio::size()            const { return history.size();}

double Portfolio::getTotalCost() const {
    double total = 0.0;
    for (const auto& ro : history)
        total += ro.result.getTotalCostForOrder();
    return total;
}

double Portfolio::getTotalQuantityFilled() const {
    double total = 0.0;
    for (const auto& ro : history)
        total += static_cast<double>(ro.result.getQuantityFilled());
    return total;
}