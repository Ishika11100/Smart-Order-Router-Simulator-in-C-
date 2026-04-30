#include "Venue.h"

Venue::Venue(const std::string& n, double s, double makerFee, double takerFee)
    : name(n), spread(s), makerFeePerShare(makerFee), takerFeePerShare(takerFee) {}

std::string Venue::getName()     const { return name;             }
double Venue::getSpread()        const { return spread;           }
double Venue::getMakerFee()      const { return makerFeePerShare; }
double Venue::getTakerFee()      const { return takerFeePerShare; }

double Venue::calculateHalfSpreadCost() const {
    return spread / 2.0;
}

// Live-spread override: if the caller has a real NBBO spread from Yahoo,
// use that instead of the hardcoded venue spread.
// All US venues trade at the NBBO by regulation (SEC Rule 611), so the
// live spread applies equally to all venues — fee is the differentiator.
double Venue::calculateHalfSpreadCostLive(double liveSpread) const {
    if (liveSpread > 0.0) return liveSpread / 2.0;
    return spread / 2.0;   // fallback to hardcoded if no live data
}

// ── Almgren-Chriss Square-Root Market Impact ─────────────────────────────────
//
// Formula: impact = price × σ_daily × √(qty / ADV)
//
// Intuition:
//   Your order size as a fraction of daily volume (qty/ADV) is called the
//   "participation rate." A 500-share order in AAPL (ADV ~80M) is a
//   participation rate of 0.000006 — trivially small, near-zero impact.
//   A 500-share order in a stock with ADV 1,000 is a 50% participation rate —
//   you are HALF the market, massive impact.
//
//   Multiplying by σ_daily captures the idea that impact is proportional to
//   how much the stock naturally moves. In a quiet stock, your impact stands
//   out less. In a volatile stock, your impact is amplified.
//
// Dimensional analysis:
//   price [$/sh] × σ_daily [dimensionless] × √(qty/ADV) [dimensionless]
//   = $/share  ✓

double Venue::calculateSlippage(double price, int qty,
                                 double dailyVol, double adv) const {
    if (adv <= 0.0) return 999999.0;  // degenerate venue — never pick this
    double participationRate = static_cast<double>(qty) / adv;
    return price * dailyVol * std::sqrt(participationRate);
}

double Venue::calculateExecutionPrice(double marketPrice, int qty,
                                       const std::string& side,
                                       double dailyVol, double adv) const {
    double halfSpread = calculateHalfSpreadCost();
    double slippage   = calculateSlippage(marketPrice, qty, dailyVol, adv);
    // BUY:  you pay above midprice (market moves against you going up)
    // SELL: you receive below midprice (market moves against you going down)
    if (side == "SELL") return marketPrice - halfSpread - slippage;
    return marketPrice + halfSpread + slippage;
}

double Venue::calculateTotalExchangeCostPerShare(double price, int qty,
                                                  double dailyVol, double adv) const {
    return calculateHalfSpreadCost()
         + calculateSlippage(price, qty, dailyVol, adv)
         + takerFeePerShare;
}