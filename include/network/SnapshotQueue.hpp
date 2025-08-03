#pragma once
#include <atomic>
#include <utility>
#include<string>

/* A very small SPSC queue specialised for our workload:
   - capacity is fixed at compile time (power-of-2 for cheap masking)
   - one producer (WS thread), one consumer (algo thread)
   - each slot holds: { symbol, bestBid , bestAsk }             */
template<std::size_t N = 1024>
class SnapshotQueue {
    static_assert((N & (N - 1)) == 0, "N must be power of two");
    struct Node { std::string sym; double bid; double ask; };

public:
    bool push(std::string s, double bid, double ask)
    {
        const auto head = head_.load(std::memory_order_relaxed);
        const auto next = (head + 1) & mask_;
        if(next == tail_.load(std::memory_order_acquire))
            return false;                               // queue full → drop
        buf_[head].sym  = s;
        buf_[head].bid = bid;
        buf_[head].ask   = ask;
        head_.store(next, std::memory_order_release);
        return true;
    }

    bool pop(std::string &s, double& bid, double& ask)
    {
        const auto tail = tail_.load(std::memory_order_relaxed);
        if(tail == head_.load(std::memory_order_acquire))
            return false;                               // queue empty
        auto& n = buf_[tail];
        s   = n.sym;  bid = n.bid;  ask = n.ask;
        tail_.store((tail + 1) & mask_, std::memory_order_release);
        return true;
    }

private:
    static constexpr std::size_t mask_ = N - 1;
    Node                         buf_[N];
    std::atomic<std::size_t>     head_{0}, tail_{0};
};
