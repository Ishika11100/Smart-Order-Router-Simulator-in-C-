#ifndef REGULATORYFEES_H
#define REGULATORYFEES_H

#include <string>
#include <algorithm>  // std::min

// RegulatoryFees: mandatory US equity market fees.
//
// These are NOT optional. Every US equity trade incurs these charges
// regardless of which broker or exchange you use. A real SOR must
// include them in total cost or the TCA numbers are wrong.
//
// Three components:
//
//  1. SEC Section 31 Transaction Fee
//     - Charged on SELL side only
//     - Calculated as: rate × (execution price × quantity) / quantity
//       = rate × execution price per share
//     - Rate: $0.0000278 per dollar of sale proceeds (2024 rate)
//     - Paid by broker to the SEC to fund market regulation
//
//  2. FINRA Trading Activity Fee (TAF)
//     - Charged on SELL side only
//     - Flat rate: $0.000145 per share, capped at $7.27 per trade
//     - The cap matters: for large orders the per-share cost shrinks
//     - Paid to FINRA to fund broker-dealer oversight
//
//  3. DTCC / NSCC Clearing Fee
//     - Charged on BOTH sides (buy and sell)
//     - ~$0.0002 per share (approximate; actual DTCC schedule is tiered)
//     - Paid to the Depository Trust & Clearing Corp. for settlement
//
// References:
//   SEC:   https://www.sec.gov/info/edgar/siccodes.htm (Section 31 rates updated annually)
//   FINRA: https://www.finra.org/filing-reporting/taf
//   DTCC:  https://www.dtcc.com/clearing-services/equities-clearing-services

class RegulatoryFees {
public:
    // ── Rate constants ────────────────────────────────────────────────────────
    static constexpr double SEC31_RATE       = 0.0000278;  // $/$ of proceeds
    static constexpr double FINRA_TAF_RATE   = 0.000145;   // $/share
    static constexpr double FINRA_TAF_CAP    = 7.27;       // $/trade
    static constexpr double DTCC_RATE        = 0.0002;     // $/share (both sides)

    // SEC Section 31 — sell side only, proportional to execution price
    static double sec31PerShare(double execPrice, const std::string& side) {
        if (side != "SELL") return 0.0;
        return execPrice * SEC31_RATE;
    }

    // FINRA TAF — sell side only, flat per-share rate with trade-level cap
    // The cap is per-trade, so per-share cost = min(rate, cap/qty)
    static double finraTafPerShare(int qty, const std::string& side) {
        if (side != "SELL") return 0.0;
        double totalTaf = std::min(FINRA_TAF_RATE * static_cast<double>(qty), FINRA_TAF_CAP);
        return totalTaf / static_cast<double>(qty);
    }

    // DTCC clearing — both sides, flat rate per share
    static double dtccPerShare() {
        return DTCC_RATE;
    }

    // Aggregate: total mandatory regulatory cost per share for one trade
    static double totalPerShare(double execPrice, int qty, const std::string& side) {
        return sec31PerShare(execPrice, side)
             + finraTafPerShare(qty, side)
             + dtccPerShare();
    }
};

#endif