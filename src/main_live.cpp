#include "Graph.hpp"
#include "BellmanFord.hpp"
#include "GraphBuilder.hpp"
#include "PriceSnapshot.hpp"
#include "LocalOrderBook.hpp"
#include "network/RestClient.hpp"
#include "network/WsClient.hpp"
#include "network/SnapshotQueue.hpp"

#include <csignal>
#include <unordered_map>
#include <iostream>
#include <chrono>
#include <memory>
#include <vector>
#include <cmath>

static std::uint64_t tick = 0;
static std::atomic<bool> stopAll{false};
void sigHandler(int){ stopAll.store(true); }

int main(int argc,char* argv[])
{
    // ---- 1. parse CLI flags (hard-coded here for brevity) ----
    std::string syms[3] = {"BTCUSDT","ETHUSDT","ETHBTC"};

    // ---- 2. seed order books from REST ----
    RestClient rest;
    LocalOrderBook ob[3];
    std::uint64_t lastId[3];
    if(curl_global_init(CURL_GLOBAL_DEFAULT)!=CURLE_OK)
        throw std::runtime_error("curl_global_init failed");

    for(int i=0;i<3;++i){
        auto snap = rest.safeFetchDepth(syms[i], 100);
        ob[i].seed(snap.bids, snap.asks);
        std::cout << "[REST] Seeded " << syms[i]
          << "  bestBid Price=" << snap.bids.front().first
          << "  bestAsk Price=" << snap.asks.front().first << '\n';
        lastId[i] = snap.lastUpdateId;
    }

    // ---- 3. start WebSocket clients (producer side) ----
    SnapshotQueue<> q;

    std::vector<std::unique_ptr<WsClient>> ws;
    for(int i=0;i<3;++i){
        // std::string url =
        //     std::string("wss://stream.binance.com:9443/ws/") +
        //     syms[i] + "@depth@100ms";
        std::string raw = syms[i];
        std::transform(raw.begin(), raw.end(), raw.begin(),
                    [](unsigned char c){ return std::tolower(c); });
        
        // stream += "@depth10@100ms";
        std::string stream  =  raw + "@depth@100ms";
        std::string url = "wss://stream.binance.com:9443/ws/" + stream ;

        // const std::string symStr = syms[i];              // lives for program lifetime
        ws.emplace_back(new WsClient(
            url, stream, lastId[i], ob[i],
            [&q,sym=  raw](const std::string&,const std::pair<double,double>& ba){
                std::cout<<"Result from web socket:"<<sym<<", "<<ba.first<<", "<<ba.second<<"\n";
                q.push(sym, ba.first, ba.second);
            }));
        ws.back()->start();
    }

    // ---- 4. Ctrl-C shutdown hook ----
    std::signal(SIGINT,  sigHandler);
    std::signal(SIGTERM, sigHandler);

    // ---- 5. algorithm loop (consumer side) ----
    std::unordered_map<std::string, std::pair<double,double>> top;
    Graph g;
    int ign = 0;
    
    while(!stopAll.load()){
        // std::cout<<tick<<":"<<'\n';
        std::string sym; double bid, ask;
        if(!q.pop(sym, bid, ask)){
            std::cout<<ign++<<" ";
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        top[sym] = {bid, ask};

        if(top.size() == 3){                       // have all three books
            auto t0 = std::chrono::steady_clock::now();

            // buildGraphFromQuotes(top, g);          // helper you already had
            Graph g = buildGraphFromQuotes(top);
            // auto cyc = bf.findNegativeCycle(g);
            auto res = findNegativeCycle(g);


            auto μs = std::chrono::duration_cast<std::chrono::microseconds>(
                         std::chrono::steady_clock::now()-t0).count();
            if(res.found){
                std::cout << "[PROFIT] ";
                for(int v: res.cycle) std::cout << ' ' << g.nameOf(v);
                std::cout << "  (" << μs << " µs)\n";
            }
            
            // if(!cyc.empty())
            //     std::cout << "[PROFIT]  " << cyc << "  (" << μs << " µs)\n";
        }
        
        if(++tick % 5 == 0) {                         // 1 line every 50 updates
            using std::chrono::system_clock;
            auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                        system_clock::now().time_since_epoch()).count();

            std::cout << "[WS " << ts << "] "
                    << sym << "  bid=" << std::fixed << std::setprecision(2)
                    << bid << "  ask=" << ask << '\n';   // '\n' flushes the line
        }
    }

    // ---- 6. graceful shutdown ----
    for(auto& p : ws) p->requestStop();
    for(auto& p : ws) if(p) p.reset();           // dtor joins thread
    std::cout << "bye\n";
    curl_global_cleanup();
    return 0;
}
