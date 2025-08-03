#pragma once
#include <string>
#include <vector>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

struct DepthSnapshot {
    std::vector<std::pair<double,double>> bids;  // {price , qty}
    std::vector<std::pair<double,double>> asks;
    std::uint64_t lastUpdateId = 0;              // seeds WS-diff filter
};

class RestClient {
public:
    explicit RestClient(const std::string& baseUrl = "https://api.binance.com");
    DepthSnapshot fetchDepth(const std::string& symbol, int limit = 1000) const;
    DepthSnapshot safeFetchDepth(const std::string& sym, int limit = 100) ;
private:
    std::string base_;
    static size_t writeCb(void* p, size_t sz, size_t nm, void* usr);
};
