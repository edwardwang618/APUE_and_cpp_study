// =============================================================================
// test_cache.cpp — correctness and performance tests
// =============================================================================
#include "market_data_cache.h"

#include <cassert>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static constexpr int64_t SEC = 1'000'000'000LL;

MarketDataEntry make_entry(int64_t time_ns, double bid, double ask) {
  MarketDataEntry e;
  e.time = time_ns;
  e.bids = {{bid, 1.0}};
  e.asks = {{ask, 1.0}};
  return e;
}

template <typename F> double bench_us(F &&f, int iterations = 1) {
  auto t0 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < iterations; ++i)
    f();
  auto t1 = std::chrono::high_resolution_clock::now();
  double total = std::chrono::duration<double, std::micro>(t1 - t0).count();
  return total / iterations;
}

#define CHECK(expr)                                                            \
  do {                                                                         \
    if (!(expr)) {                                                             \
      std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);     \
      std::exit(1);                                                            \
    }                                                                          \
  } while (0)

#define CHECK_NEAR(a, b, eps)                                                  \
  do {                                                                         \
    double _a = (a), _b = (b), _e = (eps);                                     \
    if (std::abs(_a - _b) > _e) {                                              \
      std::fprintf(stderr, "FAIL %s:%d: |%f - %f| > %f\n", __FILE__, __LINE__, \
                   _a, _b, _e);                                                \
      std::exit(1);                                                            \
    }                                                                          \
  } while (0)

// ---------------------------------------------------------------------------
// Test 1: basic insert and count
// ---------------------------------------------------------------------------
void test_basic_insert_count() {
  std::printf("  test_basic_insert_count ... ");

  MarketDataCache cache;
  CHECK(cache.count() == 0);

  int64_t t0 = 1'000'000'000'000'000'000LL;
  cache.insert(make_entry(t0, 100.0, 101.0));
  cache.insert(make_entry(t0 + 50 * SEC, 100.0, 101.5));
  cache.insert(make_entry(t0 + 100 * SEC, 100.0, 102.0));

  CHECK(cache.count() == 3);
  CHECK(cache.count_range(t0, t0 + 200 * SEC) == 3);
  CHECK(cache.count_range(t0, t0 + 50 * SEC) == 2);
  CHECK(cache.count_range(t0 - 1000 * SEC, t0 - 1 * SEC) == 0);

  std::printf("PASS\n");
}

// ---------------------------------------------------------------------------
// Test 2: min/max spread
// ---------------------------------------------------------------------------
void test_min_max_spread() {
  std::printf("  test_min_max_spread ... ");

  MarketDataCache cache;
  int64_t t0 = 1'000'000'000'000'000'000LL;

  cache.insert(make_entry(t0, 100.0, 101.0));
  cache.insert(make_entry(t0 + 10 * SEC, 100.0, 101.5));
  cache.insert(make_entry(t0 + 20 * SEC, 100.0, 102.0));

  CHECK_NEAR(cache.min_spread(t0, t0 + 30 * SEC), 1.0, 1e-9);
  CHECK_NEAR(cache.max_spread(t0, t0 + 30 * SEC), 2.0, 1e-9);
  CHECK_NEAR(cache.min_spread(t0 + 10 * SEC, t0 + 10 * SEC), 1.5, 1e-9);
  CHECK_NEAR(cache.max_spread(t0 + 10 * SEC, t0 + 10 * SEC), 1.5, 1e-9);

  std::printf("PASS\n");
}

// ---------------------------------------------------------------------------
// Test 3: exact percentiles
// ---------------------------------------------------------------------------
void test_exact_percentiles() {
  std::printf("  test_exact_percentiles ... ");

  MarketDataCache cache;
  int64_t t0 = 1'000'000'000'000'000'000LL;

  for (int i = 1; i <= 100; ++i) {
    double spread = static_cast<double>(i);
    cache.insert(make_entry(t0 + i * SEC, 100.0, 100.0 + spread));
  }

  auto [p10, p50, p90] = cache.spread_percentiles_exact(t0, t0 + 200 * SEC);

  CHECK_NEAR(p10, 10.0, 1e-9);
  CHECK_NEAR(p50, 50.0, 1e-9);
  CHECK_NEAR(p90, 90.0, 1e-9);

  std::printf("PASS\n");
}

// ---------------------------------------------------------------------------
// Test 4: approximate percentiles
// ---------------------------------------------------------------------------
void test_approx_percentiles() {
  std::printf("  test_approx_percentiles ... ");

  MarketDataCache cache(0.0, 110.0);
  int64_t t0 = 1'000'000'000'000'000'000LL;

  for (int i = 1; i <= 100; ++i) {
    double spread = static_cast<double>(i);
    cache.insert(make_entry(t0 + i * SEC, 100.0, 100.0 + spread));
  }

  auto [p10, p50, p90] = cache.spread_percentiles(t0, t0 + 200 * SEC);

  double eps = cache.hist_bin_width() + 0.01;
  CHECK_NEAR(p10, 10.0, eps);
  CHECK_NEAR(p50, 50.0, eps);
  CHECK_NEAR(p90, 90.0, eps);

  std::printf("PASS\n");
}

// ---------------------------------------------------------------------------
// Test 5: remove_up_to
// ---------------------------------------------------------------------------
void test_remove_up_to() {
  std::printf("  test_remove_up_to ... ");

  MarketDataCache cache;
  int64_t t0 = 1'000'000'000'000'000'000LL;

  cache.insert(make_entry(t0, 100.0, 101.0));
  cache.insert(make_entry(t0 + 10 * SEC, 100.0, 102.0));
  cache.insert(make_entry(t0 + 20 * SEC, 100.0, 103.0));

  CHECK(cache.count() == 3);
  cache.remove_up_to(t0 + 10 * SEC);
  CHECK(cache.count() == 1);
  CHECK_NEAR(cache.min_spread(t0, t0 + 30 * SEC), 3.0, 1e-9);

  std::printf("PASS\n");
}

// ---------------------------------------------------------------------------
// Test 6: window eviction on insert
// ---------------------------------------------------------------------------
void test_window_eviction() {
  std::printf("  test_window_eviction ... ");

  MarketDataCache cache;
  int64_t t0 = 1'000'000'000'000'000'000LL;

  cache.insert(make_entry(t0, 100.0, 101.0));
  CHECK(cache.count() == 1);

  int64_t t1 = t0 + 3601LL * SEC;
  cache.insert(make_entry(t1, 200.0, 203.0));
  CHECK(cache.count() == 1);
  CHECK_NEAR(cache.min_spread(t1, t1), 3.0, 1e-9);

  std::printf("PASS\n");
}

// ---------------------------------------------------------------------------
// Test 7: multiple entries per bucket
// ---------------------------------------------------------------------------
void test_multiple_per_bucket() {
  std::printf("  test_multiple_per_bucket ... ");

  MarketDataCache cache;
  int64_t t0 = 1'000'000'000'000'000'000LL;

  for (int i = 0; i < 5; ++i) {
    double spread = 1.0 + i * 0.5;
    cache.insert(make_entry(t0 + i * 1'000'000LL, 100.0, 100.0 + spread));
  }

  CHECK(cache.count() == 5);
  CHECK(cache.count_range(t0, t0 + 100'000'000LL) == 5);
  CHECK_NEAR(cache.min_spread(t0, t0 + 100'000'000LL), 1.0, 1e-9);
  CHECK_NEAR(cache.max_spread(t0, t0 + 100'000'000LL), 3.0, 1e-9);

  std::printf("PASS\n");
}

// ---------------------------------------------------------------------------
// Test 8: empty range queries
// ---------------------------------------------------------------------------
void test_empty_range() {
  std::printf("  test_empty_range ... ");

  MarketDataCache cache;
  int64_t t0 = 1'000'000'000'000'000'000LL;

  CHECK(cache.count_range(t0, t0 + SEC) == 0);
  CHECK(std::isnan(cache.min_spread(t0, t0 + SEC)));
  CHECK(std::isnan(cache.max_spread(t0, t0 + SEC)));

  auto [p10, p50, p90] = cache.spread_percentiles(t0, t0 + SEC);
  CHECK(std::isnan(p10));
  CHECK(std::isnan(p50));
  CHECK(std::isnan(p90));

  std::printf("PASS\n");
}

// ---------------------------------------------------------------------------
// Test 9: load from JSON file (if available)
// ---------------------------------------------------------------------------
void test_json_load(const std::string &path) {
  std::printf("  test_json_load(%s) ... ", path.c_str());

  try {
    auto cache = MarketDataCache::with_file(path, -5.0, 20.0);
    int64_t c = cache.count();
    std::printf("loaded %lld entries. ", (long long)c);
    if (c > 0) {
      int64_t huge = INT64_MAX;
      double mn = cache.min_spread(0, huge);
      double mx = cache.max_spread(0, huge);
      std::printf("min_spread=%.6f max_spread=%.6f ", mn, mx);
      auto [p10, p50, p90] = cache.spread_percentiles(0, huge);
      std::printf("p10=%.4f p50=%.4f p90=%.4f ", p10, p50, p90);
      auto [e10, e50, e90] = cache.spread_percentiles_exact(0, huge);
      std::printf("exact p10=%.6f p50=%.6f p90=%.6f ", e10, e50, e90);
    }
    std::printf("PASS\n");
  } catch (const std::exception &e) {
    std::printf("SKIP (%s)\n", e.what());
  }
}

// ---------------------------------------------------------------------------
// Performance benchmark
// ---------------------------------------------------------------------------
void benchmark_performance() {
  std::printf("\n=== Performance Benchmark ===\n");

  const int N = 1'000'000;
  const int Q = 100'000;
  int64_t t0 = 1'000'000'000'000'000'000LL;

  std::mt19937_64 rng(42);
  std::uniform_real_distribution<double> spread_dist(0.5, 8.0);
  std::uniform_int_distribution<int64_t> time_dist(0, 3500LL * SEC);

  MarketDataCache cache(0.0, 10.0);

  // --- Insertion benchmark ---
  std::vector<MarketDataEntry> entries(N);
  for (int i = 0; i < N; ++i) {
    double spread = spread_dist(rng);
    double bid = 100.0;
    entries[i] = make_entry(t0 + time_dist(rng), bid, bid + spread);
  }
  std::sort(entries.begin(), entries.end(),
            [](auto &a, auto &b) { return a.time < b.time; });

  double ins_us = bench_us(
      [&] {
        MarketDataCache c(0.0, 10.0);
        for (auto &e : entries)
          c.insert(e);
      },
      1);
  std::printf("  Insert %d entries:          %.1f ms  (%.0f ns/insert)\n", N,
              ins_us / 1000.0, ins_us * 1000.0 / N);

  for (auto &e : entries)
    cache.insert(e);
  std::printf("  Cache count: %lld\n", (long long)cache.count());

  // --- Generate random query ranges ---
  struct QueryRange {
    int64_t s, e;
  };
  std::vector<QueryRange> queries(Q);
  std::uniform_int_distribution<int64_t> qdist(0, 3000LL * SEC);
  std::uniform_int_distribution<int64_t> wdist(1LL * SEC, 600LL * SEC);
  for (int i = 0; i < Q; ++i) {
    int64_t qs = t0 + qdist(rng);
    int64_t qe = qs + wdist(rng);
    queries[i] = {qs, qe};
  }

  // --- count_range ---
  {
    volatile int64_t sink = 0;
    double us = bench_us(
        [&] {
          for (auto &q : queries)
            sink = cache.count_range(q.s, q.e);
        },
        3);
    std::printf("  count_range  (%d queries):  %.1f ms  (%.0f ns/query)\n", Q,
                us / 1000.0, us * 1000.0 / Q);
  }

  // --- min_spread ---
  {
    volatile double sink = 0;
    double us = bench_us(
        [&] {
          for (auto &q : queries)
            sink = cache.min_spread(q.s, q.e);
        },
        3);
    std::printf("  min_spread   (%d queries):  %.1f ms  (%.0f ns/query)\n", Q,
                us / 1000.0, us * 1000.0 / Q);
  }

  // --- max_spread ---
  {
    volatile double sink = 0;
    double us = bench_us(
        [&] {
          for (auto &q : queries)
            sink = cache.max_spread(q.s, q.e);
        },
        3);
    std::printf("  max_spread   (%d queries):  %.1f ms  (%.0f ns/query)\n", Q,
                us / 1000.0, us * 1000.0 / Q);
  }

  // --- spread_percentiles (approximate) ---
  {
    volatile double sink = 0;
    double us = bench_us(
        [&] {
          for (auto &q : queries) {
            auto [a, b, c] = cache.spread_percentiles(q.s, q.e);
            sink = a + b + c;
          }
        },
        3);
    std::printf(
        "  spread_pctls (%d queries):  %.1f ms  (%.0f ns/query)  [approx]\n", Q,
        us / 1000.0, us * 1000.0 / Q);
  }

  // --- spread_percentiles (exact, fewer queries since it's slower) ---
  {
    int Qe = std::min(Q, 1000);
    volatile double sink = 0;
    double us = bench_us(
        [&] {
          for (int i = 0; i < Qe; ++i) {
            auto &q = queries[i];
            auto [a, b, c] = cache.spread_percentiles_exact(q.s, q.e);
            sink = a + b + c;
          }
        },
        1);
    std::printf(
        "  spread_pctls (%d queries):  %.1f ms  (%.0f ns/query)  [exact]\n", Qe,
        us / 1000.0, us * 1000.0 / Qe);
  }
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
int main(int argc, char *argv[]) {
  std::printf("=== Correctness Tests ===\n");

  test_basic_insert_count();
  test_min_max_spread();
  test_exact_percentiles();
  test_approx_percentiles();
  test_remove_up_to();
  test_window_eviction();
  test_multiple_per_bucket();
  test_empty_range();

  std::string json_path = "market_data.json";
  if (argc > 1)
    json_path = argv[1];
  test_json_load(json_path);

  std::printf("\nAll correctness tests passed.\n");

  benchmark_performance();

  return 0;
}