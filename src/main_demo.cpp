#include "PriceSnapshot.hpp"

#include "Graph.hpp"
#include "BellmanFord.hpp"
#include <nlohmann/json.hpp>
#include<fstream>
#include<iostream>


int main(){
    nlohmann::json j;
    std::ifstream("data/snapshot_2024-07-31.json") >> j;
    std::cout<<"Json file successfully Loaded\n";
    Snapshot snap = j.get<Snapshot>();
    std::cout<<"Snapshot successfully Created\n\nEdges:\n";
    Graph g;
    for(auto& q: snap) {
        int vBase =  g.addVertex(q.base);
        int vQuote = g.addVertex(q.quote);
        g.addEdge(vQuote, vBase, std::log(q.ask));
        g.addEdge(vBase, vQuote, std::log(1.0/q.bid));
        std::cout<<vQuote<<"->"<<vBase<<":"<<std::log(q.ask)<<"\n";
        std::cout<<vBase<<"->"<<vQuote<<":"<<std::log(1.0/q.bid)<<"\n";
    }
    std::cout<<"\n\nGraph successfully Created\n";

    // Run Bellman Ford
    auto res = findNegativeCycle(g);
    std::cout<<"Arbitrage successfuly found\n";
    if(!res.found){
        std::cout<<"No arbitrage exists in the provided snapshot.\n";
        return 0;
    }

    std::cout<<"Arbitrage Path:";
    for(int v:res.cycle){
        std::cout<<' '<< g.nameOf(v);
    }
    std::cout<<"\n";

}


