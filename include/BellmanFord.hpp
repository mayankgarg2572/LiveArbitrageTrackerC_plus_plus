#pragma once
#include "Graph.hpp"
#include<optional>
#include<vector>


struct CycleResult {
    bool found;
    std:: vector<int> cycle;   // sequence of vertices Ids

};

CycleResult findNegativeCycle( const Graph& g);
