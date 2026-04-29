#ifndef VENUE_H
#define VENUE_H

#include <string>

// Venue: represents a trading exchange with its own cost parameters.
// Encapsulates spread, fee, and liquidity; exposes cost calculation methods.
class Venue {
private:
    std::string name;
    double spread;          // full bid-ask spread in dollars
    double transactionFee;  // fee per share (negative = liquidity rebate)
    int liquidity;          // available shares at the venue

public:
    Venue(const std::string& n, double s, double fee, int l);

    std::string getName()          const;
    double getSpread()             const;
    double getTransactionFee()     const;
    int getLiquidity()             const;

    double calculateHalfSpreadCost()                                        const;
    double calculateSlippage(int orderSize)                                 const;
    double calculateExecutionPrice(double marketPrice, int orderSize,
                                   const std::string& side)                 const;
    double calculateTotalCostPerShare(int orderSize)                        const;
};

#endif