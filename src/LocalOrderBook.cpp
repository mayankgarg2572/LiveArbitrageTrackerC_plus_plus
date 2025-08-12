#include "LocalOrderBook.hpp"
#include <algorithm>          // std::max
#include <utility>            // std::pair

// --- seed ------------------------------------------------------------------
// One-shot call at start-up: copy the full REST snapshot into our maps.
void LocalOrderBook::seed(const std::vector<std::pair<double,double>>& bids,
                          const std::vector<std::pair<double,double>>& asks)
{
    std::lock_guard<std::mutex> lg(mu_); 
    bids_.clear(); asks_.clear();

    for (auto& p : bids) if (p.second > 0)  bids_.emplace(p); // log-time 
    for (auto& p : asks) if (p.second > 0)  asks_.emplace(p);
}

// --- applyDiff -------------------------------------------------------------
// Merge one WebSocket depthUpdate message (absolute quantities).
// If qty==0 remove the level; else overwrite/insert.
void LocalOrderBook::applyDiff(const std::vector<std::pair<double,double>>& bidDiff,
                               const std::vector<std::pair<double,double>>& askDiff)
{
    std::lock_guard<std::mutex> lg(mu_);

    for (auto& p : bidDiff) // bids are stored in DESC order
        (p.second == 0.0) ? bids_.erase(p.first) // erase-by-key is O(log N)      :contentReference[oaicite:2]{index=2}
                          : bids_[p.first] = p.second; 

    for (auto& p : askDiff)     // asks in ASC (default) order
        (p.second == 0.0) ? asks_.erase(p.first)
                          : asks_[p.first] = p.second;
}

// --- bestBidAsk ------------------------------------------------------------
// Return top-of-book prices used by the Bellman-Ford graph.
std::pair<double,double> LocalOrderBook::bestBidAsk() const
{
    std::lock_guard<std::mutex> lg(mu_);
    double bestBid = bids_.empty() ? 0.0 : bids_.begin()->first;   // begin() → max price because of std::greater
    double bestAsk = asks_.empty() ? 0.0 : asks_.begin()->first;   // begin() → min price (default order)
    return {bestBid, bestAsk};
}
