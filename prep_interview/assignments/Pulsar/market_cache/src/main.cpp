// =============================================================================
// main.cpp — quick demo of MarketDataCache
// =============================================================================
#include "market_data_cache.h"
#include <climits>
#include <cstdio>
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
  std::string path = "market_data.json";
  if (argc > 1)
    path = argv[1];

  std::printf("Loading %s ...\n", path.c_str());
  auto cache = MarketDataCache::with_file(path, -5.0, 20.0);

  std::printf("Entries loaded: %lld\n", (long long)cache.count());

  int64_t lo = 0;
  int64_t hi = INT64_MAX;

  std::printf("count_range:  %lld\n", (long long)cache.count_range(lo, hi));
  std::printf("min_spread:   %.10f\n", cache.min_spread(lo, hi));
  std::printf("max_spread:   %.10f\n", cache.max_spread(lo, hi));

  {
    auto [p10, p50, p90] = cache.spread_percentiles(lo, hi);
    std::printf("spread_percentiles (approx): p10=%.6f  p50=%.6f  p90=%.6f\n",
                p10, p50, p90);
  }
  {
    auto [p10, p50, p90] = cache.spread_percentiles_exact(lo, hi);
    std::printf("spread_percentiles (exact):  p10=%.6f  p50=%.6f  p90=%.6f\n",
                p10, p50, p90);
  }

  std::printf("\nHistogram config: min=%.2f  max=%.2f  bin_width=%.4f  "
              "bins=%d  max_error=+/-%.4f\n",
              cache.hist_min(), cache.hist_max(), cache.hist_bin_width(),
              MarketDataCache::NUM_HIST_BINS, cache.hist_bin_width());

  return 0;
}