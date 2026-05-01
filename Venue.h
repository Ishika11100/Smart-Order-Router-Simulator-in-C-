#ifndef VENUE_H
#define VENUE_H

#include <string>
#include <cmath>

// Venue represents one stock exchange (NYSE, NASDAQ, IEX, CBOE).
// each venue has its own spread, maker fee, and taker fee.
//
// maker/taker explained quick:
//   taker = market order that removes liquidity from the book (you pay a fee)
//   maker = limit order that adds liquidity to the book (you get a rebate)
// since we only use market orders in this project, taker fee always applies.
//
// the slippage formula here is Almgren-Chriss (not the simple linear one
// from the midway version). uses sqrt(qty/ADV) which is the industry standard.
// basically: bigger order relative to daily volume = more price impact.

class Venue {
private:
    std::string name;
    double spread;           // full bid-ask spread in dollars
    double makerFeePerShare; // rebate for resting orders (negative = exchange pays you)
    double takerFeePerShare; // fee for market orders (always positive)

public:
    Venue(const std::string& n, double spread, double makerFee, double takerFee);

    std::string getName()           const;
    double      getSpread()         const;
    double      getMakerFee()       const;
    double      getTakerFee()       const;
    double      getTransactionFee() const { return takerFeePerShare; } // convenience alias

    double calculateHalfSpreadCost() const;

    // Almgren-Chriss sqrt model -- needs the stock's daily vol and ADV
    // which is why we added MarketData as a separate struct in v2
    double calculateSlippage(double price, int qty, double dailyVol, double adv) const;

    double calculateExecutionPrice(double marketPrice, int qty,
                                   const std::string& side,
                                   double dailyVol, double adv) const;

    // exchange-side cost only (spread + slippage + taker fee)
    // regulatory fees are handled separately in SmartOrderRouter
    double calculateTotalExchangeCostPerShare(double price, int qty,
                                              double dailyVol, double adv) const;

    // uses live NBBO spread from Yahoo if available, falls back to hardcoded
    double calculateHalfSpreadCostLive(double liveSpread) const;
};

#endif