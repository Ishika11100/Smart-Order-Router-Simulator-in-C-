#include "Venue.h"

Venue::Venue(const std::string& n, double s, double fee, int l)
    : name(n), spread(s), transactionFee(fee), liquidity(l) {}

std::string Venue::getName()          const { return name;           }
double      Venue::getSpread()        const { return spread;         }
double      Venue::getTransactionFee()const { return transactionFee; }
int         Venue::getLiquidity()     const { return liquidity;      }

double Venue::calculateHalfSpreadCost() const {
    return spread / 2.0;
}

// Slippage = k * (orderSize / liquidity)
// Larger orders relative to liquidity incur higher slippage.
double Venue::calculateSlippage(int orderSize) const {
    const double k = 0.10;
    if (liquidity <= 0) return 999999.0;   // penalty for invalid liquidity
    return k * (static_cast<double>(orderSize) / static_cast<double>(liquidity));
}

// BUY:  pay market + half-spread + slippage
// SELL: receive market - half-spread - slippage
double Venue::calculateExecutionPrice(double marketPrice, int orderSize,
                                      const std::string& side) const {
    double halfSpread = calculateHalfSpreadCost();
    double slippage   = calculateSlippage(orderSize);
    if (side == "SELL") return marketPrice - halfSpread - slippage;
    return marketPrice + halfSpread + slippage;
}

// Total friction cost per share above (below) the reference market price
double Venue::calculateTotalCostPerShare(int orderSize) const {
    return calculateHalfSpreadCost() + calculateSlippage(orderSize) + transactionFee;
}