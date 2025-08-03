#pragma once
#include<unordered_map>
#include<vector>
#include<string>

struct Edge
{
    int u, v; double w;
};

class Graph {
    public:
        int addVertex(const std::string &sym);
        void addEdge( int u, int v, double weight);
        const std:: vector<Edge> & edges() const {return E;}
        int V() const {return vertices; }
        const std::string& nameOf(int id) const;
    
    private:
        int vertices = 0;
        std::unordered_map<std:: string, int> idOf;
        std::vector<std::string>names;
        std:: vector<Edge>E;

};
