#ifndef MARKETDATA_H
#define MARKETDATA_H

#include <string>
#include <cmath>

// per-symbol market data needed for the slippage formula.
// we fetch this from Yahoo Finance at startup instead of hardcoding it
// so the numbers actually reflect current market conditions.
//
// historical fields (computed from 20 days of daily closes):
//   dailyVolatility  -- 1-day log return std dev, e.g. 0.0151 for AAPL
//   adv              -- 20-day average daily volume in shares
//
// live quote fields (from Yahoo 1-minute bars today):
//   livePrice  -- last trade price
//   liveBid    -- current NBBO best bid
//   liveAsk    -- current NBBO best ask
//   bidSize    -- shares at bid (in round lots, multiply by 100 for actual shares)
//   askSize    -- shares at ask

struct MarketData {
    std::string symbol;

    // historical
    double dailyVolatility = 0.0;
    double adv             = 0.0;

    // live quote -- these stay 0.0 outside market hours since Yahoo stops sending them
    double livePrice = 0.0;
    double liveBid   = 0.0;
    double liveAsk   = 0.0;
    int    bidSize   = 0;
    int    askSize   = 0;

    // live spread in dollars. returns 0.0 if bid/ask arent available
    // (Venue code checks for 0.0 and falls back to hardcoded spread)
    double liveSpread() const {
        if (liveAsk > 0.0 && liveBid > 0.0 && liveAsk > liveBid)
            return liveAsk - liveBid;
        return 0.0;
    }

    // convenience factory -- pass annual vol and it converts to daily for you
    // daily = annual / sqrt(252 trading days)
    static MarketData fromAnnual(const std::string& sym,
                                  double annualVol,
                                  double advShares) {
        MarketData md;
        md.symbol          = sym;
        md.dailyVolatility = annualVol / std::sqrt(252.0);
        md.adv             = advShares;
        return md;
    }
};

#endif