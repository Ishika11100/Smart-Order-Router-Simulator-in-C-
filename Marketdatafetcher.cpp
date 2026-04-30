#include "MarketDataFetcher.h"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <vector>
#include <cmath>
#include <numeric>
#include <iostream>
#include <iomanip>
#include <stdexcept>

using json = nlohmann::json;

// ── libcurl write callback ────────────────────────────────────────────────────
static size_t writeCallback(void* contents, size_t size, size_t nmemb,
                             std::string* output) {
    output->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

// ── Shared HTTP GET helper ────────────────────────────────────────────────────
std::string MarketDataFetcher::httpGet(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl)
        throw std::runtime_error("curl_easy_init() failed");

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL,           url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,     &response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT,
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,        15L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
        throw std::runtime_error(std::string("HTTP request failed: ")
                                  + curl_easy_strerror(res));
    return response;
}

// ── Step 1: Historical vol + ADV ─────────────────────────────────────────────
//
// Endpoint: Yahoo Finance v8 chart, daily bars, last 40 calendar days
// Computes:
//   - 20-day realised daily volatility from log close-to-close returns
//   - 20-day average daily volume

void MarketDataFetcher::fetchHistorical(MarketData& md) {
    std::string url =
        "https://query1.finance.yahoo.com/v8/finance/chart/"
        + md.symbol + "?interval=1d&range=40d";

    json j = json::parse(httpGet(url));

    if (!j["chart"]["error"].is_null())
        throw std::runtime_error(
            j["chart"]["error"]["description"].get<std::string>());

    auto& results = j["chart"]["result"];
    if (results.is_null() || results.empty())
        throw std::runtime_error("No chart data for " + md.symbol);

    auto& quote   = results[0]["indicators"]["quote"][0];
    auto& closes  = quote["close"];
    auto& volumes = quote["volume"];

    // Collect valid (non-null) close prices and volumes
    std::vector<double> closePrices, volSeries;
    for (size_t i = 0; i < closes.size(); ++i) {
        if (!closes[i].is_null() && !volumes[i].is_null()) {
            closePrices.push_back(closes[i].get<double>());
            volSeries.push_back(volumes[i].get<double>());
        }
    }

    if (closePrices.size() < 5)
        throw std::runtime_error("Too few trading days for " + md.symbol);

    // 20-day realised vol from log returns (Bessel-corrected sample std dev)
    size_t N        = std::min(closePrices.size(), size_t(21));
    size_t startIdx = closePrices.size() - N;

    std::vector<double> logReturns;
    for (size_t i = startIdx + 1; i < closePrices.size(); ++i)
        if (closePrices[i-1] > 0.0 && closePrices[i] > 0.0)
            logReturns.push_back(std::log(closePrices[i] / closePrices[i-1]));

    if (logReturns.empty())
        throw std::runtime_error("Cannot compute returns for " + md.symbol);

    double mean = std::accumulate(logReturns.begin(), logReturns.end(), 0.0)
                  / static_cast<double>(logReturns.size());
    double var  = 0.0;
    for (double r : logReturns) var += (r - mean) * (r - mean);
    var /= static_cast<double>(logReturns.size() - 1);
    md.dailyVolatility = std::sqrt(var);

    // 20-day ADV
    size_t vStart = volSeries.size() >= 20 ? volSeries.size() - 20 : 0;
    double sumVol = 0.0;
    size_t vCount = 0;
    for (size_t i = vStart; i < volSeries.size(); ++i) { sumVol += volSeries[i]; ++vCount; }
    md.adv = vCount > 0 ? sumVol / static_cast<double>(vCount) : 5'000'000.0;
}

// ── Step 2: Live quote — price, bid, ask, sizes ───────────────────────────────
//
// Endpoint: Yahoo Finance v8 chart with 1-minute bars for today.
// The most recent bar's close gives the last trade price.
// The summary block gives bid, ask, bidSize, askSize.
//
// Yahoo v8 chart response (1m/1d) structure:
// {
//   "chart": { "result": [{
//     "meta": {
//       "regularMarketPrice": 189.30,    ← last trade
//       "bid": 189.28,                   ← NBBO bid
//       "ask": 189.32,                   ← NBBO ask  (not always present)
//     },
//     "indicators": { "quote": [{ "close": [...] }] }
//   }]}
// }
//
// Note: bid/ask in Yahoo's chart meta are not always populated outside
// regular market hours. We parse them defensively and fall back gracefully.

void MarketDataFetcher::fetchLiveQuote(MarketData& md) {
    // 1-minute bars for today gives us the freshest last-trade price
    std::string url =
        "https://query1.finance.yahoo.com/v8/finance/chart/"
        + md.symbol + "?interval=1m&range=1d";

    json j = json::parse(httpGet(url));

    if (!j["chart"]["error"].is_null())
        throw std::runtime_error(
            j["chart"]["error"]["description"].get<std::string>());

    auto& results = j["chart"]["result"];
    if (results.is_null() || results.empty())
        throw std::runtime_error("No live quote data for " + md.symbol);

    auto& meta = results[0]["meta"];

    // ── Live price ────────────────────────────────────────────────────────────
    // Prefer "regularMarketPrice" (last trade) over "chartPreviousClose"
    if (!meta["regularMarketPrice"].is_null())
        md.livePrice = meta["regularMarketPrice"].get<double>();
    else if (!meta["chartPreviousClose"].is_null())
        md.livePrice = meta["chartPreviousClose"].get<double>();

    // ── Live bid / ask ────────────────────────────────────────────────────────
    // Yahoo populates these during regular market hours.
    // Outside hours they may be absent — we leave them 0.0 (liveSpread() = 0.0)
    // and the Venue falls back to its hardcoded spread.
    if (meta.contains("bid") && !meta["bid"].is_null())
        md.liveBid = meta["bid"].get<double>();

    if (meta.contains("ask") && !meta["ask"].is_null())
        md.liveAsk = meta["ask"].get<double>();

    // bidSize and askSize come in round lots (×100 shares)
    if (meta.contains("bidSize") && !meta["bidSize"].is_null())
        md.bidSize = meta["bidSize"].get<int>();

    if (meta.contains("askSize") && !meta["askSize"].is_null())
        md.askSize = meta["askSize"].get<int>();

    // Sanity check: if bid >= ask something is wrong, zero them out
    if (md.liveBid > 0.0 && md.liveAsk > 0.0 && md.liveBid >= md.liveAsk) {
        md.liveBid = 0.0;
        md.liveAsk = 0.0;
    }
}

// ── Public entry point ────────────────────────────────────────────────────────

MarketData MarketDataFetcher::fetch(const std::string& symbol) {
    MarketData md;
    md.symbol = symbol;

    fetchHistorical(md);   // sets dailyVolatility, adv
    fetchLiveQuote(md);    // sets livePrice, liveBid, liveAsk, bidSize, askSize

    // Print a live summary so the user can see real data hitting the model
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "  [Live] " << symbol
              << "  price=$"  << md.livePrice;
    if (md.liveBid > 0.0)
        std::cout << "  bid=$"  << md.liveBid
                  << " (" << (md.bidSize * 100) << "sh)"
                  << "  ask=$"  << md.liveAsk
                  << " (" << (md.askSize * 100) << "sh)"
                  << "  spread=$" << md.liveSpread();
    else
        std::cout << "  (bid/ask unavailable outside market hours)";

    std::cout << "  20d_vol=" << (md.dailyVolatility * std::sqrt(252.0) * 100.0) << "%"
              << "  20d_ADV=" << static_cast<long long>(md.adv) << "\n";

    return md;
}