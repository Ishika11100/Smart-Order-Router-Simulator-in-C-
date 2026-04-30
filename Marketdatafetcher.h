#ifndef MARKETDATAFETCHER_H
#define MARKETDATAFETCHER_H

#include "MarketData.h"
#include <string>

// MarketDataFetcher: fetches all real market data needed by the SOR.
//
// Two separate HTTP requests per symbol:
//
//   1. Historical (Yahoo v8 chart, range=40d, interval=1d)
//      → computes 20-day realised daily volatility from log close returns
//      → computes 20-day ADV from actual traded volume
//
//   2. Live quote (Yahoo v8 chart, range=1d, interval=1m — most recent bar)
//      → livePrice : last trade / regular market price
//      → liveBid   : current NBBO best bid
//      → liveAsk   : current NBBO best ask
//      → bidSize / askSize : depth at best bid/ask in round lots (×100 shares)
//
// The two calls are merged into one MarketData struct and returned together.
//
// Dependencies (MSYS2 MinGW64):
//   pacman -S mingw-w64-x86_64-curl
//   pacman -S mingw-w64-x86_64-nlohmann-json

class MarketDataFetcher {
public:
    // Fetches complete MarketData (historical + live quote) for one symbol.
    // Throws std::runtime_error on network failure, bad symbol, or parse error.
    static MarketData fetch(const std::string& symbol);

private:
    // Step 1: fetch 40 days of daily OHLCV → compute vol + ADV
    static void fetchHistorical(MarketData& md);

    // Step 2: fetch live quote → fill livePrice, liveBid, liveAsk, sizes
    static void fetchLiveQuote(MarketData& md);

    // Shared libcurl helper: performs one HTTPS GET, returns response body
    static std::string httpGet(const std::string& url);
};

#endif