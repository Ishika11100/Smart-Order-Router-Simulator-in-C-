/*
 * Smart Order Router Simulator  –  main.cpp
 * OOP I  (22:839:614)  Spring 2026
 * Group: Ishika Patel, Emmanuel Ayinoluwa, George Marcu
 *
 * Demonstrates:
 *   - Inheritance / polymorphism  (Order* → MarketOrder)
 *   - Encapsulation across 7 classes
 *   - STL containers (vector, shared_ptr)
 *   - File input / output  (orders.txt  →  results.txt)
 *   - Exception handling   (try / catch blocks)
 *   - Dual routing strategy comparison
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <stdexcept>
#include <iomanip>

#include "MarketOrder.h"
#include "Venue.h"
#include "SmartOrderRouter.h"
#include "Portfolio.h"
#include "PerformanceAnalyzer.h"

// ── File loader ───────────────────────────────────────────────────────────────
// Parses orders.txt; returns a vector of heap-allocated Order objects.
// Uses polymorphism: all entries become MarketOrder, stored as shared_ptr<Order>.

std::vector<std::shared_ptr<Order>> loadOrdersFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Cannot open '" + filename + "'");

    std::vector<std::shared_ptr<Order>> orders;
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string tok, sym, side;
        int id = 0, qty = 0;
        double price = 0.0;

        try {
            std::getline(ss, tok,  ','); id    = std::stoi(tok);
            std::getline(ss, sym,  ',');
            std::getline(ss, side, ',');
            std::getline(ss, tok,  ','); qty   = std::stoi(tok);
            std::getline(ss, tok,  ','); price = std::stod(tok);
        } catch (const std::exception& e) {
            throw std::runtime_error("Malformed line in orders file: \"" + line + "\"");
        }

        if (qty <= 0 || price <= 0.0)
            throw std::runtime_error("Invalid quantity or price in: \"" + line + "\"");

        // Polymorphic construction – stored as base class pointer
        orders.push_back(std::make_shared<MarketOrder>(id, sym, side, qty, price));
    }

    if (orders.empty())
        throw std::runtime_error("No valid orders found in '" + filename + "'");

    return orders;
}

// ── Utility ───────────────────────────────────────────────────────────────────

static void printBanner(const std::string& text) {
    const int W = 84;
    std::cout << "\n" << std::string(W, '=') << "\n";
    // Centre the text
    int pad = (W - static_cast<int>(text.size())) / 2;
    if (pad > 0) std::cout << std::string(pad, ' ');
    std::cout << text << "\n";
    std::cout << std::string(W, '=') << "\n";
}

// ── main ──────────────────────────────────────────────────────────────────────

int main() {

    // ── 1.  Register trading venues ──────────────────────────────────────────
    //        name     spread   fee      liquidity
    SmartOrderRouter router;
    router.addVenue(Venue("NYSE",   0.04,  0.003, 10000));
    router.addVenue(Venue("NASDAQ", 0.02,  0.005,  5000));
    router.addVenue(Venue("IEX",    0.01, -0.001,  2000));

    // ── 2.  Load orders (file first; fall back to hardcoded set) ─────────────
    std::vector<std::shared_ptr<Order>> orders;
    try {
        orders = loadOrdersFromFile("orders.txt");
        std::cout << "Loaded " << orders.size() << " order(s) from orders.txt\n";
    } catch (const std::exception& e) {
        std::cerr << "[Warning] " << e.what() << " – using built-in test orders.\n";
        orders.push_back(std::make_shared<MarketOrder>(1, "AAPL",  "BUY",  500, 100.00));
        orders.push_back(std::make_shared<MarketOrder>(2, "MSFT",  "BUY",  300, 250.00));
        orders.push_back(std::make_shared<MarketOrder>(3, "TSLA",  "SELL", 200, 180.00));
        orders.push_back(std::make_shared<MarketOrder>(4, "AMZN",  "BUY", 1000, 175.00));
        orders.push_back(std::make_shared<MarketOrder>(5, "GOOGL", "SELL", 150, 155.00));
    }

    // ── 3.  Portfolios – one per strategy ────────────────────────────────────
    Portfolio smartPortfolio("Smart Router");
    Portfolio baselinePortfolio("Baseline (always NYSE)");

    // ── 4.  Route every order through both strategies ────────────────────────
    printBanner("SMART ORDER ROUTER SIMULATOR");

    for (const auto& orderPtr : orders) {
        std::cout << "\n";
        orderPtr->describe();                       // polymorphic call
        router.printVenueComparison(*orderPtr);     // full cost table

        // Smart routing: picks cheapest venue
        try {
            ExecutionResult smartResult = router.routeOrder(*orderPtr);
            std::cout << "\n  [SMART ROUTE] -> Best Venue\n";
            smartResult.printSummary(*orderPtr);
            smartPortfolio.addRoutedOrder(orderPtr, smartResult);
        } catch (const std::exception& e) {
            std::cerr << "  [ERROR] Smart routing failed: " << e.what() << "\n";
        }

        // Baseline routing: always sends to first registered venue (NYSE)
        try {
            ExecutionResult baselineResult = router.routeOrderBaseline(*orderPtr);
            std::cout << "\n  [BASELINE ROUTE] -> " << baselineResult.getVenueName() << "\n";
            baselineResult.printSummary(*orderPtr);
            baselinePortfolio.addRoutedOrder(orderPtr, baselineResult);
        } catch (const std::exception& e) {
            std::cerr << "  [ERROR] Baseline routing failed: " << e.what() << "\n";
        }

        std::cout << "  " << std::string(82, '-') << "\n";
    }

    // ── 5.  Performance reports ───────────────────────────────────────────────
    PerformanceAnalyzer smartAnalyzer(smartPortfolio);
    PerformanceAnalyzer baselineAnalyzer(baselinePortfolio);

    smartAnalyzer.printReport();
    baselineAnalyzer.printReport();

    // ── 6.  Strategy comparison summary ──────────────────────────────────────
    double smartTotal    = smartPortfolio.getTotalCost();
    double baselineTotal = baselinePortfolio.getTotalCost();
    double savings       = baselineTotal - smartTotal;
    double savingsPct    = (baselineTotal != 0.0) ? (savings / baselineTotal) * 100.0 : 0.0;

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "\n=== Strategy Comparison ===\n";
    std::cout << "  Smart Router Total Cost     : $" << smartTotal    << "\n";
    std::cout << "  Baseline Total Cost         : $" << baselineTotal << "\n";
    std::cout << "  Savings via Smart Routing   : $" << savings
              << "  (" << std::setprecision(2) << savingsPct << "%)\n";

    // ── 7.  Write full results to file ────────────────────────────────────────
    try {
        std::ofstream outFile("results.txt");
        if (!outFile.is_open())
            throw std::runtime_error("Cannot open results.txt for writing.");

        outFile << "SMART ORDER ROUTER SIMULATOR – RESULTS\n";
        outFile << std::string(60, '=') << "\n\n";
        outFile << "--- Order Execution Log (Smart Router) ---\n\n";

        for (const auto& ro : smartPortfolio.getHistory())
            ro.result.writeToStream(outFile, *ro.order);

        smartAnalyzer.writeReport(outFile);
        baselineAnalyzer.writeReport(outFile);

        outFile << std::fixed << std::setprecision(4);
        outFile << "=== Strategy Comparison ===\n";
        outFile << "  Smart Router Total Cost   : $" << smartTotal    << "\n";
        outFile << "  Baseline Total Cost       : $" << baselineTotal << "\n";
        outFile << "  Savings via Smart Routing : $" << savings
                << "  (" << std::setprecision(2) << savingsPct << "%)\n";

        std::cout << "\nResults written to results.txt\n";
    } catch (const std::exception& e) {
        std::cerr << "[Warning] Could not write results file: " << e.what() << "\n";
    }

    return 0;
}