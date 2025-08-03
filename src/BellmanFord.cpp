#include "BellmanFord.hpp"
#include<algorithm>
#include<iostream>


CycleResult findNegativeCycle(const Graph &g){
    const int N  =  g.V();

    std::vector<double> dist(N+1, 0.0);  // initialising all dist  = 0.0
    
    std::vector<int> parent(N+1, -1);
    
    for(int i=1;i<N;i++){
        for (auto &e: g.edges()){
            if (dist[e.u] +e.w < dist[e.v] ){
                dist[e.v]  = dist[e.u] +e.w;
                parent[e.v] =  e.u;  
            }
        }
    }
    std::cout<<"First loop done\n";

    int x  = -1;
    for (auto &e: g.edges()){
        if(dist[e.u] + e.w < dist[e.v] ){
            parent[e.v] = e.u;
            x = e.v;
        }
    }
    std::cout<<"Second loop done\n";

    if(x==-1) return {false, {}};  // no arbitrage

    for(int i=0;i<N;i++){
        x  = parent[x];
    }

    std::cout<<"Third loop done\n";

    std:: vector<int> cycle;
    for(int v  = x;v>-1 && v<N+1; v = parent[v]){
        cycle.push_back(v);
        if(v == x && cycle.size() > 1){
            break;
        }
    }
    std::cout<<"Fourth loop done\n";
    std::reverse(cycle.begin(), cycle.end());

    std::cout<<"Fifth loop done\n";
    return {true, cycle};

}
