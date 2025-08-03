#pragma once
#include <string>
#include<vector>
#include<nlohmann/json.hpp>




struct Quote{
    std::string base, quote;
    double bid, ask;
};

using Snapshot = std::vector<Quote>;


inline void from_json(const nlohmann::json &j, Quote& q) {
    j.at("base").get_to(q.base);
    j.at("quote").get_to(q.quote);
    j.at("bid").get_to(q.bid);
    j.at("ask").get_to(q.ask);

}

inline void to_json(nlohmann:: json &j, const Quote& q){
    j = nlohmann::json{
        {"base", q.base},
        {"quote", q.quote},
        {"bid", q.bid},
        {"ask", q.ask}
    };
}
