#ifndef REGULATORYFEES_H
#define REGULATORYFEES_H

#include <string>
#include <algorithm>

// mandatory US equity market fees -- you cant avoid these no matter which
// exchange you use. every single US equity trade pays them.
//
// three components:
//
//   SEC Section 31  (sell side only)
//     rate * execution price per share. funds SEC operations.
//     rate changes annually -- using 2024 rate of $0.0000278 per dollar of proceeds.
//
//   FINRA TAF  (sell side only)
//     $0.000145 per share, but capped at $7.27 per trade.
//     the cap matters for large orders -- once you hit 50,000 shares
//     the per-share cost starts shrinking. funds FINRA oversight.
//
//   DTCC clearing  (both buy and sell)
//     ~$0.0002 per share flat. funds settlement infrastructure.
//
// we made all methods static because theres no state to store,
// its just math. call them directly: RegulatoryFees::totalPerShare(...)

class RegulatoryFees {
public:
    static constexpr double SEC31_RATE     = 0.0000278;
    static constexpr double FINRA_TAF_RATE = 0.000145;
    static constexpr double FINRA_TAF_CAP  = 7.27;
    static constexpr double DTCC_RATE      = 0.0002;

    // SEC31 -- sell only, scales with execution price
    static double sec31PerShare(double execPrice, const std::string& side) {
        if (side != "SELL") return 0.0;
        return execPrice * SEC31_RATE;
    }

    // FINRA TAF -- sell only, flat rate with a per-trade cap
    static double finraTafPerShare(int qty, const std::string& side) {
        if (side != "SELL") return 0.0;
        double total = std::min(FINRA_TAF_RATE * static_cast<double>(qty), FINRA_TAF_CAP);
        return total / static_cast<double>(qty);
    }

    // DTCC -- both sides, flat rate
    static double dtccPerShare() {
        return DTCC_RATE;
    }

    // combined total of all three
    static double totalPerShare(double execPrice, int qty, const std::string& side) {
        return sec31PerShare(execPrice, side)
             + finraTafPerShare(qty, side)
             + dtccPerShare();
    }
};

#endif