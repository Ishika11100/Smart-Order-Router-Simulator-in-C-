#include "Performanceanalyzer.h"
#include <iostream>
#include <iomanip>

PerformanceAnalyzer::PerformanceAnalyzer(const Portfolio& p) : portfolio(p) {}

double PerformanceAnalyzer::getAverageExecutionPrice() const {
    const auto& h = portfolio.getHistory();
    if (h.empty()) return 0.0;
    double sum = 0.0;
    for (const auto& ro : h) sum += ro.result.getExecutionPrice();
    return sum / static_cast<double>(h.size());
}

double PerformanceAnalyzer::getTotalFees() const {
    double total = 0.0;
    for (const auto& ro : portfolio.getHistory())
        total += ro.result.getFeePerShare() * static_cast<double>(ro.order->getQuantity());
    return total;
}

double PerformanceAnalyzer::getTotalSlippage() const {
    double total = 0.0;
    for (const auto& ro : portfolio.getHistory())
        total += ro.result.getSlippagePerShare() * static_cast<double>(ro.order->getQuantity());
    return total;
}

// Simplified: all market orders assumed fully filled (no partial fill model).
double PerformanceAnalyzer::getFillRate() const {
    return 100.0;
}

// Implementation shortfall = total cost above the reference market price
// across all orders in this portfolio.
double PerformanceAnalyzer::getImplementationShortfall() const {
    return portfolio.getTotalCost();
}

// ── Reporting helpers ─────────────────────────────────────────────────────────

static void writeMetrics(std::ostream& out, const std::string& label,
                          size_t count, double avgExec, double fees,
                          double slippage, double fill, double is,
                          double totalCost) {
    out << std::fixed << std::setprecision(4);
    out << "=== Performance Report: " << label << " ===\n";
    out << "  Orders Executed         : " << count     << "\n";
    out << "  Avg Execution Price     : $" << avgExec  << "\n";
    out << "  Total Transaction Fees  : $" << fees     << "\n";
    out << "  Total Slippage Cost     : $" << slippage << "\n";
    out << std::setprecision(2);
    out << "  Fill Rate               : " << fill << "%\n";
    out << std::setprecision(4);
    out << "  Implementation Short.   : $" << is        << "\n";
    out << "  Total Execution Cost    : $" << totalCost << "\n";
}

void PerformanceAnalyzer::printReport() const {
    std::cout << "\n";
    writeMetrics(std::cout,
                 portfolio.getStrategyName(),
                 portfolio.size(),
                 getAverageExecutionPrice(),
                 getTotalFees(),
                 getTotalSlippage(),
                 getFillRate(),
                 getImplementationShortfall(),
                 portfolio.getTotalCost());
}

void PerformanceAnalyzer::writeReport(std::ostream& out) const {
    writeMetrics(out,
                 portfolio.getStrategyName(),
                 portfolio.size(),
                 getAverageExecutionPrice(),
                 getTotalFees(),
                 getTotalSlippage(),
                 getFillRate(),
                 getImplementationShortfall(),
                 portfolio.getTotalCost());
    out << "\n";
}