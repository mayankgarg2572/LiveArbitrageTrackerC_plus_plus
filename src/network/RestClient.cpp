#include "network/RestClient.hpp"
#include <sstream>
#include <chrono>
#include <thread>

RestClient::RestClient(const std::string& baseUrl) : base_(baseUrl) {}

/* static */ size_t RestClient::writeCb(void* ptr,size_t sz,size_t nm,void* usr){
    auto* buf = static_cast<std::string*>(usr);
    buf->append(static_cast<char*>(ptr), sz * nm);
    return sz * nm;
}

DepthSnapshot RestClient::fetchDepth(const std::string& symbol,int limit) const {
    std::ostringstream url;
    url << base_ << "/api/v3/depth?symbol=" << symbol << "&limit=" << limit;

    CURL* curl = curl_easy_init();
    if(!curl) throw std::runtime_error("curl_easy_init failed");

    std::string body;
    curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 2500L);   // was 1000
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 5000L);   // new: 5 s cap
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 5L);  // abort if <100 B/s
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 100L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &RestClient::writeCb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "arb-finder/0.1");

    CURLcode rc = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if(rc != CURLE_OK)
        throw std::runtime_error(curl_easy_strerror(rc));

    auto j = nlohmann::json::parse(body);                              // :contentReference[oaicite:2]{index=2}

    DepthSnapshot snap;
    snap.lastUpdateId = j.at("lastUpdateId").get<std::uint64_t>();     // :contentReference[oaicite:3]{index=3}
    for(auto& row : j.at("bids"))
        snap.bids.emplace_back(std::stod(row[0].get<std::string>()),
                               std::stod(row[1].get<std::string>()));
    for(auto& row : j.at("asks"))
        snap.asks.emplace_back(std::stod(row[0].get<std::string>()),
                               std::stod(row[1].get<std::string>()));

    return snap;
}

DepthSnapshot RestClient::safeFetchDepth(const std::string& sym, int limit) 
{
    for(int attempt=0; attempt<2; ++attempt) {
        try { return fetchDepth(sym, limit); }
        catch(const std::runtime_error& e){
            if(attempt==0) std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    throw std::runtime_error("REST depth failed twice for " + sym);
}