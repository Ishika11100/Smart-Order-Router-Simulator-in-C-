#ifndef VENUE_H
#define VENUE_H

#include <string>
#include <cmath>

// Venue: a trading exchange with realistic cost parameters.
//
// Two upgrades from the original:
//
//  1. Maker / Taker fee structure
//     Exchanges use a "maker-taker" model. If your order RESTS in the book
//     waiting (limit order = "maker"), the exchange pays you a rebate.
//     If your order IMMEDIATELY executes against resting liquidity (market
//     order = "taker"), you pay a fee.
//     Real-world rates (approximate, 2024):
//       NYSE:   maker -$0.0020/sh  taker +$0.0030/sh
//       NASDAQ: maker -$0.0020/sh  taker +$0.0030/sh
//       IEX:    maker  $0.0000/sh  taker +$0.0009/sh  (IEX's model is special)
//       CBOE:   maker -$0.0032/sh  taker +$0.0028/sh
//     Since this simulator uses MarketOrders exclusively, the taker fee
//     always applies. The maker fee is stored for future LimitOrder support.
//
//  2. Almgren-Chriss square-root market impact (slippage)
//     Old model: slippage = k × (qty / liquidity)       ← LINEAR, unrealistic
//     New model: slippage = price × σ_daily × √(qty/ADV) ← SQRT, industry standard
//
//     Why square-root?
//       Empirically, price impact does NOT scale linearly with order size.
//       Doubling order size roughly doubles your pressure on the book, but the
//       book also deepens as you go. The square-root relationship has been
//       validated across asset classes and markets (Almgren et al. 2005,
//       Gatheral 2010). Every serious TCA/OMS system uses this or a variant.
//
//     Parameters:
//       price     – execution reference price (in $)
//       qty       – number of shares in the order
//       dailyVol  – 1-day return standard deviation of the stock (e.g. 0.0157)
//       adv       – average daily volume of the stock in shares (e.g. 80,000,000)

class Venue {
private:
    std::string name;
    double spread;             // full bid-ask spread in dollars (midpoint reference)
    double makerFeePerShare;   // exchange fee for resting orders (rebate = negative)
    double takerFeePerShare;   // exchange fee for aggressive orders (always positive)

public:
    Venue(const std::string& n, double spread,
          double makerFee, double takerFee);

    std::string getName()            const;
    double      getSpread()          const;
    double      getMakerFee()        const;
    double      getTakerFee()        const;

    // Convenience: market orders are always takers
    double      getTransactionFee()  const { return takerFeePerShare; }

    // Half the bid-ask spread paid to cross the market
    double calculateHalfSpreadCost() const;
    double calculateHalfSpreadCostLive(double liveSpread) const;  // uses NBBO if available

    // Almgren-Chriss square-root market impact:
    //   impact ($/share) = price × dailyVol × sqrt(qty / ADV)
    double calculateSlippage(double price, int qty,
                             double dailyVol, double adv) const;

    // Actual execution price after crossing spread and moving the market
    double calculateExecutionPrice(double marketPrice, int qty,
                                   const std::string& side,
                                   double dailyVol, double adv) const;

    // Exchange friction cost per share = halfSpread + slippage + takerFee
    // Does NOT include regulatory fees (computed separately in SmartOrderRouter)
    double calculateTotalExchangeCostPerShare(double price, int qty,
                                              double dailyVol, double adv) const;
};

#endif