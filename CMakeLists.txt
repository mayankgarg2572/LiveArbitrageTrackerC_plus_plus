cmake_minimum_required(VERSION 3.20)
project(arb_live LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)


find_package(CURL          CONFIG REQUIRED) 
find_package(nlohmann_json CONFIG REQUIRED)  
find_package(Threads       REQUIRED)  


add_executable(arb_live
    src/main_live.cpp
    src/network/RestClient.cpp
    src/network/WsClient.cpp
    src/GraphBuilder.cpp
    src/Graph.cpp
    src/BellmanFord.cpp
    src/LocalOrderBook.cpp)

target_sources(arb_live PRIVATE src/GraphBuilder.cpp)

target_include_directories(arb_live PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)


target_link_libraries(arb_live PRIVATE
    CURL::libcurl              
    Threads::Threads
    nlohmann_json::nlohmann_json)