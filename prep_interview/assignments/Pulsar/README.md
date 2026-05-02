# Market Data Cache

## How to Build and Run

```bash
cd Edward_Wang
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j
./market_cache ../market_data.json
./tests ../market_data.json
```

## Thoughts

The key insight is that linear scans won't cut it on a trading hot path, so everything needs to be pre-indexed. I bucket timestamps into 100ms intervals (36,000 buckets for one hour) in a circular buffer, then layer a segment tree on top for O(log n) count/min/max queries. For percentiles, I use Fenwick trees over histogram bins — this gives approximate results (error ≤ bin width, default ±1.0) but runs in microseconds instead of milliseconds. An exact fallback using nth_element is also provided. Thread safety uses a shared_mutex so concurrent reads don't block each other.


## Tools and AI Usage
I used Claude (Anthropic) as a coding assistant. The design decisions (segment tree, Fenwick-backed histograms, circular buffer, approximate vs exact percentile tradeoff) were my own reasoning based on the latency requirements. I used Claude to help implement the design, handle boilerplate (JSON parsing, CMake, test scaffolding), and debug build issues.

## Prompts
Provided the full problem statement and API spec, asked for a C++ implementation of an in-memory market data cache using time-bucketing, segment trees for range min/max/count, and Fenwick trees over histogram bins for fast approximate percentiles.