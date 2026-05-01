#ifndef MARKETDATAFETCHER_H
#define MARKETDATAFETCHER_H

#include "MarketData.h"
#include <string>

// fetches real market data from Yahoo Finance at program startup.
// makes two HTTP requests per symbol using libcurl:
//
//   call 1 -- 40 days of daily OHLCV (historical)
//     computes 20-day realized volatility from log close-to-close returns
//     computes 20-day average daily volume
//
//   call 2 -- 1-minute bars for today (live quote)
//     extracts current price, bid, ask, bidSize, askSize
//     outside market hours Yahoo stops sending bid/ask so those stay 0.0
//
// both results get merged into one MarketData struct and returned.
//
// dependencies (install once via MSYS2):
//   pacman -S mingw-w64-x86_64-curl
//   pacman -S mingw-w64-x86_64-nlohmann-json

class MarketDataFetcher {
public:
    // main entry point -- call this once per symbol at startup
    // throws std::runtime_error if network completely fails (caller catches it)
    static MarketData fetch(const std::string& symbol);

private:
    static void fetchHistorical(MarketData& md);  // fills vol + adv
    static void fetchLiveQuote(MarketData& md);   // fills price, bid, ask
    static std::string httpGet(const std::string& url); // shared curl helper
};

#endif