#ifndef MARKETDATA_H
#define MARKETDATA_H

#include <string>
#include <cmath>

// MarketData: all per-symbol parameters the SOR needs to make routing decisions.
//
// v3 adds live quote fields (livePrice, liveBid, liveAsk, bidSize, askSize)
// fetched at runtime from Yahoo Finance's quote endpoint.
//
// Why live bid-ask matters:
//   In US equity markets all venues must trade at the NBBO (National Best Bid
//   and Offer) by law — this is SEC Rule 611 (Order Protection Rule). So the
//   spread you see on Yahoo IS the real spread you'll pay on any exchange.
//   Using live bid-ask means slippage and spread costs reflect today's actual
//   market conditions rather than a hardcoded guess.
//
// Why live price matters:
//   The reference price in orders.txt is a placeholder. For real cost modeling
//   the execution price formula needs to start from where the stock actually IS
//   right now, not where it was when you typed the order file.

struct MarketData {
    std::string symbol;

    // ── Historical (computed from 20 days of close prices) ────────────────────
    double dailyVolatility;   // 1-day log-return std dev  e.g. 0.0157 = 1.57%/day
    double adv;               // 20-day average daily volume in shares

    // ── Live quote (fetched from Yahoo Finance quote endpoint) ────────────────
    double livePrice  = 0.0;  // last trade price / regular market price
    double liveBid    = 0.0;  // current best bid (NBBO bid)
    double liveAsk    = 0.0;  // current best ask (NBBO ask)
    int    bidSize    = 0;    // shares available at bid  (in lots of 100)
    int    askSize    = 0;    // shares available at ask  (in lots of 100)

    // Derived: live NBBO spread in dollars
    double liveSpread() const {
        if (liveAsk > 0.0 && liveBid > 0.0 && liveAsk > liveBid)
            return liveAsk - liveBid;
        return 0.0;   // fallback: caller uses venue's hardcoded spread
    }

    // Convenience factory: converts annualised vol to daily (no live data)
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