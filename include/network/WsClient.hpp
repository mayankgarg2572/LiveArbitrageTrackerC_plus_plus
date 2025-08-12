#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <curl/curl.h>
#include "LocalOrderBook.hpp"

/* Push freshly-merged bestBid/Ask for every symbol.
   Caller (main_live.cpp) will feed these doubles into SnapshotQueue.   */
using BookSnapshotPush =
    std::function<void(const std::string &sym,
                       const std::pair<double, double> &best)>;

class WsClient
{
public:
    WsClient(const std::string &url,
             const std::string &symbol,
             std::uint64_t lastIdSeed,
             LocalOrderBook &ob,
             BookSnapshotPush push);

    void start();       // spawns the WS thread
    void requestStop(); // sets the atomic flag
    ~WsClient();        // joins the thread

private:
    void run(); // thread main-loop
    void handleFrame(const std::string &json);

    std::string url_, symbol_;
    LocalOrderBook &book_;
    BookSnapshotPush push_;
    std::atomic<bool> stop_{false};
    std::uint64_t nextExpected_{0}; // prev ‘u’ + 1
    std::thread th_;
    bool synced_{false};    //  true after first good event
};
