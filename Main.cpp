/*
 * Smart Order Router Simulator  –  main.cpp  (v4 – Live Prices + Order Splitting)
 * OOP I  (22:839:614)  Spring 2026
 * Group: Ishika Patel, Emmanuel Ayinoluwa, George Marcu
 *
 * Upgrades over v3:
 *   - Live bid/ask prices from Yahoo Finance (NBBO spread used in cost model)
 *   - Live stock price used as reference price (overrides orders.txt price)
 *   - OrderSplitter: automatically detects large orders via participation rate
 *     threshold (1% of ADV) and allocates across venues to minimize impact
 *   - Market orders only (no limit/stop complexity)
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <stdexcept>
#include <iomanip>
#include <unordered_map>
#include <set>
#include <cmath>

#include "MarketOrder.h"
#include "Venue.h"
#include "SmartOrderRouter.h"
#include "Portfolio.h"
#include "PerformanceAnalyzer.h"
#include "MarketData.h"
#include "MarketDataFetcher.h"
#include "OrderSplitter.h"
#include "RegulatoryFees.h"

// ── Fallback market data ──────────────────────────────────────────────────────
static std::unordered_map<std::string, MarketData> buildFallbackMap() {
    return {
        { "AAPL",  MarketData::fromAnnual("AAPL",  0.25,  80'000'000) },
        { "MSFT",  MarketData::fromAnnual("MSFT",  0.22,  25'000'000) },
        { "TSLA",  MarketData::fromAnnual("TSLA",  0.55, 100'000'000) },
        { "AMZN",  MarketData::fromAnnual("AMZN",  0.28,  60'000'000) },
        { "GOOGL", MarketData::fromAnnual("GOOGL", 0.24,  25'000'000) },
    };
}

// ── File loader ───────────────────────────────────────────────────────────────
static std::vector<std::shared_ptr<Order>>
loadOrdersFromFile(const std::string& filename) {
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
        } catch (...) {
            throw std::runtime_error("Malformed line: \"" + line + "\"");
        }
        if (qty <= 0 || price <= 0.0)
            throw std::runtime_error("Invalid qty/price in: \"" + line + "\"");
        orders.push_back(std::make_shared<MarketOrder>(id, sym, side, qty, price));
    }
    if (orders.empty())
        throw std::runtime_error("No valid orders in '" + filename + "'");
    return orders;
}

// ── Live market data builder ──────────────────────────────────────────────────
static std::unordered_map<std::string, MarketData>
buildLiveMarketDataMap(const std::vector<std::shared_ptr<Order>>& orders) {
    std::set<std::string> symbols;
    for (const auto& o : orders) symbols.insert(o->getSymbol());

    auto fallback = buildFallbackMap();
    std::unordered_map<std::string, MarketData> mdMap;

    std::cout << "\nFetching live market data from Yahoo Finance...\n";
    for (const std::string& sym : symbols) {
        try {
            mdMap.emplace(sym, MarketDataFetcher::fetch(sym));
        } catch (const std::exception& e) {
            std::cerr << "  [Warning] Live fetch failed for '" << sym
                      << "': " << e.what() << "\n";
            if (fallback.count(sym) > 0) {
                std::cerr << "  [Fallback] Using hardcoded 2024 figures for '"
                          << sym << "'\n";
                mdMap.emplace(sym, fallback.at(sym));
            } else {
                mdMap.emplace(sym, MarketData::fromAnnual(sym, 0.30, 5'000'000));
            }
        }
    }
    std::cout << "Market data ready for " << mdMap.size() << " symbol(s).\n";
    return mdMap;
}

// ── Utility ───────────────────────────────────────────────────────────────────

// ── Simulate executing one VenueSlice and return its ExecutionResult ──────────
// Used when the splitter has already decided how much goes to each venue.
static ExecutionResult executeSlice(const VenueSlice& slice,
                                     const Order& originalOrder,
                                     const std::vector<Venue>& venues,
                                     const MarketData& md) {
    // Find the matching Venue object
    const Venue* v = nullptr;
    for (const auto& venue : venues)
        if (venue.getName() == slice.venueName) { v = &venue; break; }
    if (!v) throw std::runtime_error("Venue not found: " + slice.venueName);

    const double price = (md.livePrice > 0.0)
                         ? md.livePrice : originalOrder.getMarketPrice();
    const std::string& side = originalOrder.getSide();

    // Use live NBBO spread if available
    double halfSpread = (md.liveSpread() > 0.0)
                        ? md.liveSpread() / 2.0
                        : v->calculateHalfSpreadCost();

    double slippage   = v->calculateSlippage(price, slice.quantity,
                                              md.dailyVolatility, md.adv);
    double exchFee    = v->getTakerFee();
    double execPrice  = (side == "SELL")
                        ? price - halfSpread - slippage
                        : price + halfSpread + slippage;
    double regFee     = RegulatoryFees::totalPerShare(execPrice, slice.quantity, side);
    double totalPerSh = halfSpread + slippage + exchFee + regFee;
    double totalCost  = totalPerSh * static_cast<double>(slice.quantity);

    return ExecutionResult(slice.venueName, execPrice, halfSpread, slippage,
                           exchFee, regFee, totalPerSh, totalCost, slice.quantity);
}

// ── main ──────────────────────────────────────────────────────────────────────
int main() {

    // ── 1. Register trading venues ────────────────────────────────────────────
    SmartOrderRouter router;
    std::vector<Venue> venueList = {
        Venue("NYSE",    0.04,  -0.0020,  0.0030),
        Venue("NASDAQ",  0.02,  -0.0020,  0.0030),
        Venue("IEX",     0.01,   0.0000,  0.0009),
        Venue("CBOE",    0.015, -0.0032,  0.0028),
    };
    for (const auto& v : venueList) router.addVenue(v);

    // ── 2. Load orders ────────────────────────────────────────────────────────
    std::vector<std::shared_ptr<Order>> orders;
    try {
        orders = loadOrdersFromFile("orders.txt");
        std::cout << "Loaded " << orders.size() << " order(s) from orders.txt\n";
    } catch (const std::exception& e) {
        std::cerr << "[Warning] " << e.what() << " – using built-in test orders.\n";
        // Mix of small and large orders to demonstrate splitting
        orders.push_back(std::make_shared<MarketOrder>(1, "AAPL",  "BUY",    500, 100.00));
        orders.push_back(std::make_shared<MarketOrder>(2, "MSFT",  "BUY",    300, 250.00));
        orders.push_back(std::make_shared<MarketOrder>(3, "TSLA",  "SELL",   200, 180.00));
        orders.push_back(std::make_shared<MarketOrder>(4, "AMZN",  "BUY",  1000, 175.00));
        orders.push_back(std::make_shared<MarketOrder>(5, "GOOGL", "SELL",   150, 155.00));
        // Large orders — these WILL trigger splitting
        orders.push_back(std::make_shared<MarketOrder>(6, "AAPL",  "BUY", 900000, 100.00));
        orders.push_back(std::make_shared<MarketOrder>(7, "TSLA",  "SELL",1100000, 180.00));
    }

    // ── 3. Fetch live market data ─────────────────────────────────────────────
    auto mdMap = buildLiveMarketDataMap(orders);

    // ── 4. Portfolios ──────────────────────────────────────────────────────────
    Portfolio smartPortfolio("Smart Router (with splitting)");
    Portfolio baselinePortfolio("Baseline (always NYSE, no splitting)");

    // ── 5. Route every order ──────────────────────────────────────────────────
    std::cout << "\nSMART ORDER ROUTER SIMULATOR v4\n";

    for (const auto& orderPtr : orders) {
        const std::string& sym = orderPtr->getSymbol();
        const MarketData&  md  = mdMap.at(sym);

        // Override reference price with live price if available
        double refPrice = (md.livePrice > 0.0)
                          ? md.livePrice : orderPtr->getMarketPrice();

        std::cout << "\n";

        std::cout << std::fixed << std::setprecision(4);
        std::cout << "order " << orderPtr->getOrderId()
                  << "  " << orderPtr->getSymbol()
                  << "  " << orderPtr->getSide()
                  << "  qty=" << orderPtr->getQuantity()
                  << "  live_price=$" << refPrice;
        if (md.liveSpread() > 0.0)
            std::cout << "  bid=$" << md.liveBid << "  ask=$" << md.liveAsk;
        std::cout << "\n";

        // ── Splitting analysis ────────────────────────────────────────────────
        SplitPlan plan = OrderSplitter::split(*orderPtr, venueList, md);
        OrderSplitter::printPlan(plan, *orderPtr, md);

        // ── Smart routing (with splitting) ────────────────────────────────────
        std::cout << "  smart_route:\n";
        double smartTotalCost = 0.0;

        for (const auto& slice : plan.slices) {
            try {
                ExecutionResult sr = executeSlice(slice, *orderPtr, venueList, md);
                std::cout << "  -> " << slice.venueName
                          << "  " << slice.quantity << " shares\n";
                sr.printSummary(*orderPtr);
                smartPortfolio.addRoutedOrder(orderPtr, sr);
                smartTotalCost += sr.getTotalCostForOrder();
            } catch (const std::exception& e) {
                std::cerr << "  [ERROR] Slice execution: " << e.what() << "\n";
            }
        }

        if (plan.slices.size() > 1)
            std::cout << "  [SPLIT TOTAL COST]: $" << smartTotalCost << "\n";

        // ── Baseline routing (always NYSE, whole order) ───────────────────────
        std::cout << "  baseline_route (NYSE):\n";
        try {
            ExecutionResult br = router.routeOrderBaseline(*orderPtr, md);
            br.printSummary(*orderPtr);
            baselinePortfolio.addRoutedOrder(orderPtr, br);
        } catch (const std::exception& e) {
            std::cerr << "  [ERROR] Baseline routing: " << e.what() << "\n";
        }

        std::cout << "\n";
    }

    // ── 6. Performance reports ─────────────────────────────────────────────────
    PerformanceAnalyzer smartAnalyzer(smartPortfolio);
    PerformanceAnalyzer baselineAnalyzer(baselinePortfolio);
    smartAnalyzer.printReport();
    baselineAnalyzer.printReport();

    // ── 7. Strategy comparison ─────────────────────────────────────────────────
    double smartTotal    = smartPortfolio.getTotalCost();
    double baselineTotal = baselinePortfolio.getTotalCost();
    double savings       = baselineTotal - smartTotal;
    double savingsPct    = (baselineTotal != 0.0)
                           ? (savings / baselineTotal) * 100.0 : 0.0;

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "\n=== Strategy Comparison ===\n";
    std::cout << "  Smart Router Total Cost   : $" << smartTotal    << "\n";
    std::cout << "  Baseline Total Cost       : $" << baselineTotal << "\n";
    std::cout << "  Savings via Smart Routing : $" << savings
              << "  (" << std::setprecision(2) << savingsPct << "%)\n";

    // ── 8. Write results file ──────────────────────────────────────────────────
    try {
        std::ofstream outFile("results.txt");
        if (!outFile.is_open())
            throw std::runtime_error("Cannot open results.txt for writing.");

        outFile << "SMART ORDER ROUTER SIMULATOR v4 – RESULTS\n";
        outFile << "(Live Market Data + Order Splitting)\n";
        outFile << std::string(70, '=') << "\n\n";

        outFile << "--- Live Market Data Used ---\n";
        for (const auto& kv : mdMap) {
            outFile << std::fixed << std::setprecision(4);
            outFile << "  " << kv.first
                    << "  price=$"    << kv.second.livePrice
                    << "  bid=$"      << kv.second.liveBid
                    << "  ask=$"      << kv.second.liveAsk
                    << "  spread=$"   << kv.second.liveSpread()
                    << "  daily_vol=" << kv.second.dailyVolatility
                    << "  annual_vol="
                    << (kv.second.dailyVolatility * std::sqrt(252.0) * 100.0) << "%"
                    << "  ADV=" << static_cast<long long>(kv.second.adv) << "\n";
        }
        outFile << "\n--- Order Execution Log (Smart Router) ---\n\n";
        for (const auto& ro : smartPortfolio.getHistory())
            ro.result.writeToStream(outFile, *ro.order);

        smartAnalyzer.writeReport(outFile);
        baselineAnalyzer.writeReport(outFile);

        outFile << std::fixed << std::setprecision(6);
        outFile << "=== Strategy Comparison ===\n";
        outFile << "  Smart Router Total Cost   : $" << smartTotal    << "\n";
        outFile << "  Baseline Total Cost       : $" << baselineTotal << "\n";
        outFile << "  Savings via Smart Routing : $" << savings
                << "  (" << std::setprecision(2) << savingsPct << "%)\n";

        std::cout << "\nResults written to results.txt\n";
    } catch (const std::exception& e) {
        std::cerr << "[Warning] Could not write results: " << e.what() << "\n";
    }

    return 0;
}