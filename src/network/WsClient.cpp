#include "network/WsClient.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
WsClient::WsClient(const std::string &url,
                   const std::string &sym,
                   std::uint64_t lastIdSeed,
                   LocalOrderBook &ob,
                   BookSnapshotPush push)
    : url_(url), symbol_(sym), book_(ob), push_(std::move(push)),
      nextExpected_(lastIdSeed + 1) {}

WsClient::~WsClient()
{
    requestStop();
    if (th_.joinable())
        th_.join();
}
void WsClient::requestStop() { stop_.store(true); }
void WsClient::start() { th_ = std::thread(&WsClient::run, this); }


void WsClient::run()
{
    CURL *easy = curl_easy_init();
    curl_easy_setopt(easy, CURLOPT_URL, url_.c_str());
    curl_easy_setopt(easy, CURLOPT_CONNECT_ONLY, 2L); // WebSocket mode
    // curl_easy_setopt(easy, CURLOPT_VERBOSE, 1L);clear

    curl_easy_setopt(easy, CURLOPT_STDERR, stderr);              // optional: pipe to file
    curl_easy_setopt(easy, CURLOPT_WS_OPTIONS, CURLWS_RAW_MODE); // let you send
    // curl_ws_send(easy, CURLWS_TEXT, "ping", 4, NULL);
    if (curl_easy_perform(easy) != CURLE_OK) // connect phase
        throw std::runtime_error("WS connect failed");

    while (!stop_.load())
    {
        /* ---- non-blocking recv ---- */
        char buf[16 * 1024];
        size_t rlen;
        const struct curl_ws_frame *meta;
        CURLcode rc = curl_ws_recv(easy, buf, sizeof(buf), &rlen, &meta);

        if (rc == CURLE_AGAIN)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }
        if (rc)
        {
            break;
        } // network error ⇒ caller will resync

        if (meta->flags & CURLWS_TEXT)
        {
            handleFrame({buf, rlen});
        }
    }
    curl_easy_cleanup(easy);
}

void WsClient::handleFrame(const std::string &data)
{
    std::cout << "\n\n[WS RAW] " << data.substr(0, 300) << "...\n\n";
    auto j = nlohmann::json::parse(data);
    if (j.value("e", "") != "depthUpdate" || !j.contains("U") || !j.contains("u"))
    {
        std::cout << "\n\nIncorrect stream\n\n";
        return;
    }
    // U/u = first/last update-id  ,  pu = previous u
    std::uint64_t U = j.at("U").get<std::uint64_t>();
    std::uint64_t u = j.at("u").get<std::uint64_t>();
    std::uint64_t pu = j.value("pu", 0ULL); // present on Spot & Futures
    std::cout << "\n\nWS update for " << symbol_
              << ": U=" << U << ", u=" << u
              << ", expected=" << nextExpected_
              << ", pu=" << pu << "\n\n";

    if(!synced_) {
        if(u < nextExpected_) return;  // keep discarding pre-snapshot
        if(U <= nextExpected_ && u >= nextExpected_) {
            synced_ = true;   // first good event: proceed
        } else {
            return;  // still waiting for a bracketing event
        }
    } else {
        /* after sync, keep normal gap detection */
        if(U != nextExpected_) {                // pu would be even better
            std::cerr << "["<<symbol_<<"] gap after sync, restarting\n";
            stop_.store(true);
            return;
        }
    }

    // apply bid/ask deltas (absolute qty)
    std::vector<std::pair<double, double>> bDiff, aDiff;
    for (auto &row : j.at("b"))
        bDiff.emplace_back(std::stod(row[0].get<std::string>()),
                           std::stod(row[1].get<std::string>()));
    for (auto &row : j.at("a"))
        aDiff.emplace_back(std::stod(row[0].get<std::string>()),
                           std::stod(row[1].get<std::string>()));
    book_.applyDiff(bDiff, aDiff);

    nextExpected_ = u + 1;

    // push top-of-book out to SnapshotQueue via lambda
    push_(symbol_, book_.bestBidAsk());
}
