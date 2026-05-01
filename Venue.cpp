#include "Venue.h"

Venue::Venue(const std::string& n, double s, double makerFee, double takerFee)
    : name(n), spread(s), makerFeePerShare(makerFee), takerFeePerShare(takerFee) {}

std::string Venue::getName()     const { return name;             }
double Venue::getSpread()        const { return spread;           }
double Venue::getMakerFee()      const { return makerFeePerShare; }
double Venue::getTakerFee()      const { return takerFeePerShare; }

double Venue::calculateHalfSpreadCost() const {
    // you always lose half the spread when you cross the market.
    // buy at ask = midprice + half spread. sell at bid = midprice - half spread.
    return spread / 2.0;
}

double Venue::calculateHalfSpreadCostLive(double liveSpread) const {
    // if we have a real live spread from Yahoo, use that instead of the hardcoded one.
    // outside market hours liveSpread comes in as 0.0 so we fall back to hardcoded.
    if (liveSpread > 0.0) return liveSpread / 2.0;
    return spread / 2.0;
}

double Venue::calculateSlippage(double price, int qty,
                                 double dailyVol, double adv) const {
    // Almgren-Chriss square root market impact model:
    //   impact = price * daily_volatility * sqrt(order_size / avg_daily_volume)
    //
    // the sqrt is key -- doubling order size only multiplies impact by ~1.41
    // not by 2. thats what makes it more realistic than the linear version we had before.
    //
    // if adv is somehow zero or negative we return a huge penalty so this
    // venue never gets picked
    if (adv <= 0.0) return 999999.0;
    double participationRate = static_cast<double>(qty) / adv;
    return price * dailyVol * std::sqrt(participationRate);
}

double Venue::calculateExecutionPrice(double marketPrice, int qty,
                                       const std::string& side,
                                       double dailyVol, double adv) const {
    double halfSpread = calculateHalfSpreadCost();
    double slippage   = calculateSlippage(marketPrice, qty, dailyVol, adv);
    // buying: price goes up (market moves against you)
    // selling: price goes down (also moves against you)
    if (side == "SELL") return marketPrice - halfSpread - slippage;
    return marketPrice + halfSpread + slippage;
}

double Venue::calculateTotalExchangeCostPerShare(double price, int qty,
                                                  double dailyVol, double adv) const {
    // this is just the exchange side -- regulatory fees get added on top of this
    // in SmartOrderRouter. kept them separate so we can report them individually.
    return calculateHalfSpreadCost()
         + calculateSlippage(price, qty, dailyVol, adv)
         + takerFeePerShare;
}