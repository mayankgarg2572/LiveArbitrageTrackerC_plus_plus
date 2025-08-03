#include "GraphBuilder.hpp"
#include <cmath>            // std::log
#include <stdexcept>        // std::runtime_error

// -- hard-coded trio used by main_live.cpp ----------------------------------
static const char* MARKETS[3] = {"BTCUSDT", "ETHUSDT", "ETHBTC"};

Graph buildGraphFromQuotes(
    const std::unordered_map<std::string,
                             std::pair<double,double>>& q)
{
    Graph g;

    auto add = [&](const std::string& base,
                   const std::string& quote)
    {
        const auto it = q.find(base + quote);
        if(it == q.end())
            throw std::runtime_error("missing market "+base+quote);

        const double bid = it->second.first;   // sell BASE → QUOTE
        const double ask = it->second.second;  // buy  BASE ← QUOTE

        int vBase  = g.addVertex(base);
        int vQuote = g.addVertex(quote);

        g.addEdge(vQuote, vBase , std::log( ask      ));  // buy leg
        g.addEdge(vBase , vQuote, std::log(1.0 / bid));  // sell leg
    };

    // Feed the three triangular legs Binance streams at 100 ms
    add("BTC","USDT");
    add("ETH","USDT");
    add("ETH","BTC");

    return g;
}
