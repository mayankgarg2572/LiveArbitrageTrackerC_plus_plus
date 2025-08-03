#pragma once
#include <unordered_map>
#include <string>
#include "Graph.hpp"

/*  ------------------------------------------------------------------
    Build a log-weighted graph from best-bid / best-ask quotes.

    - q maps "BASEQUOTE"  → {bid , ask}
        bid = price you get when _selling_ BASE for QUOTE
        ask = price you pay when _buying_ BASE with QUOTE

    - For every market we insert two directed edges
        QUOTE → BASE   weight =  ln( ask )        (buy BASE)
        BASE  → QUOTE  weight =  ln(1 / bid)      (sell BASE)

    A negative-sum cycle in the resulting graph is a risk-free
    arbitrage opportunity.  --------------------------------------------------*/
Graph buildGraphFromQuotes(
    const std::unordered_map<std::string,
                             std::pair<double,double>>& q);
