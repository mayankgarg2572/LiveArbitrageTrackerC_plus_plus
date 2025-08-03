#pragma once
#include<map>
#include<vector>
#include<mutex>
#include <utility>

class LocalOrderBook{
    public:
        void seed(const std::vector<std::pair<double, double>>& bid, const std::vector<std::pair<double, double>>& ask );
        
        void applyDiff(const std::vector<std::pair<double, double>>& bidDiff, const std::vector<std::pair<double, double>>& askDiff);

        std::pair<double, double> bestBidAsk() const;
    
    private:
        std::map<double, double, std::greater<double>> bids_;
        std::map<double, double> asks_;

        mutable std::mutex mu_;
};