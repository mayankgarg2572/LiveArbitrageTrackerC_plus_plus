## Live Arbitrage Tracker C++ <br>*(`mayankgarg2572-livearbitragetrackerc_plus_plus`)*  

A **high-performance, single-process arbitrage detector** that streams live order-book data from Binance, keeps local depth for three triangular markets (`BTC-USDT`, `ETH-USDT`, `ETH-BTC`) and executes a logarithmic-weight Bellman–Ford pass on every update to surface risk-free cycles in micro-seconds.

---

### Table of Contents
1. [Key Capabilities](#key-capabilities)  
2. [Directory Layout](#directory-layout)  
3. [Prerequisites](#prerequisites)  
4. [Build & Install](#build--install)  
5. [Running the Binaries](#running-the-binaries)  
6. [Configuration](#configuration)  
7. [Internals — How It Works](#internals—how-it-works)  
8. [Troubleshooting](#troubleshooting)  
9. [License](#license)

---

### Key Capabilities
| Function | Detail |
|-----------|--------|
| **REST seeding** | Fetches the full depth snapshot once and primes an in-memory order-book. |
| **WebSocket merging** | Applies incremental `depthUpdate` frames with gap detection and automatic re-sync. |
| **SPSC snapshot queue** | Lock-free, cache-friendly ring buffer connects the network thread (producer) to the algo thread (consumer). |
| **Graph construction** | Converts top-of-book quotes to a log-weighted directed graph in \< 5 µs. |
| **Arbitrage search** | Classic Bellman–Ford negative-cycle detection; returns explicit vertex path. |
| **Deterministic latency** | End-to-end (WS frame → console) measured \< 80 µs on a Ryzen 7 5800U. |

---

### Directory Layout
```text
<repo>/
├── CMakeLists.txt            # cross-platform build script
├── config/                   # user-editable runtime files
│   └── exchanges.json        # (reserved) – extend for multi-venue support
├── data/                     # sample snapshots for offline tests
│   └── snapshot_2024-07-31.json
├── include/                  # public headers
│   ├── *.hpp
│   └── network/...
├── src/                      # translation units
│   ├── main_demo.cpp         # offline replay of JSON snapshot
│   ├── main_live.cpp         # production live tracker
│   └── network/...
└── docs/                     # *(create for extra material)*
```

### Prerequisites
| Tool | Minimum Version | Notes |
|------|----------------|-------|
| **CMake** | 3.20 | Ninja backend recommended |
| **C++ Compiler** | C++20 | Tested with *GCC 13* (`mingw-w64-ucrt-x86_64` tool-chain) |
| **libcurl**       | ≥ 7.88          | Built-in WebSocket support (`curl_ws_*` API)           |
| **nlohmann/json** | ≥ 3.11          | Header-only                                            |
| **OpenSSL libs**  | —               | Runtime dependency of libcurl                          |

**Windows / MSYS2 UCRT64 quick-install**
```bash
pacman -S --needed \
  mingw-w64-ucrt-x86_64-toolchain \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-libcurl \
  mingw-w64-ucrt-x86_64-nlohmann-json
```


### Build & Install
#### 1 · Release build
```bash
git clone https://github.com/<your-fork>/mayankgarg2572-livearbitragetrackerc_plus_plus.git
cd mayankgarg2572-livearbitragetrackerc_plus_plus
mkdir -p build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j          # produces ./arb_live
```

#### 2 · Debug build (optional)
```bash
mkdir -p ../build-debug && cd ../build-debug
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
gdb ./arb_live          # or VS Code F5
```

VS Code shortcuts
• Ctrl + Shift + B — configure / compile
• F5 — launch debugger with symbols