#include "Graph.hpp"

void Graph::addEdge(int u, int v, double w){
    E.push_back({u, v, w});
}


int  Graph:: addVertex(const std::string& sym){
    auto it = idOf.find(sym);
    if(it!= idOf.end()) return it->second;

    int id = vertices++;
    idOf[sym] = id;
    names.push_back(sym);
    return id;
}

const std::string& Graph::nameOf(int id) const {
    return names[id];
}