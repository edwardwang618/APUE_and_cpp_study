#pragma once

#include <array>
#include <climits>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <tuple>
#include <vector>

// ---- Data types -------------------------------------------------------------

struct PriceLevel {
  double price;
  double amount;
};

struct MarketDataEntry {
  int64_t time; // UTC epoch in nanoseconds
  std::vector<PriceLevel> bids;
  std::vector<PriceLevel> asks;

  // Spread = lowest ask price - highest bid price.
  double compute_spread() const;
};

// ---- Cache ------------------------------------------------------------------

class MarketDataCache {
public:
  // --- Configuration constants ---
  static constexpr int64_t BUCKET_NS = 100'000'000LL; // 100 ms
  static constexpr int NUM_BUCKETS = 36'000;          // 1 hour
  static constexpr int SEG_TREE_SIZE = 4 * NUM_BUCKETS;
  static constexpr int NUM_HIST_BINS = 100;

  // --- Construction ---
  explicit MarketDataCache(double hist_min = -5.0, double hist_max = 95.0);

  // Move constructor / assignment (shared_mutex is not movable, so we
  // construct a fresh one — the moved-from object must not be used).
  MarketDataCache(MarketDataCache &&other) noexcept;
  MarketDataCache &operator=(MarketDataCache &&other) noexcept;

  // Not copyable (contains mutex).
  MarketDataCache(const MarketDataCache &) = delete;
  MarketDataCache &operator=(const MarketDataCache &) = delete;

  // Load entries from a JSON file (format: {"market_data_entries": [...]}).
  static MarketDataCache with_file(const std::string &file_path,
                                   double hist_min = -5.0,
                                   double hist_max = 95.0);

  // --- Mutators ---
  void insert(const MarketDataEntry &data);
  void remove_up_to(int64_t time);

  // --- Queries (hot-path) ---
  int64_t count() const;
  int64_t count_range(int64_t start_time, int64_t end_time) const;

  std::tuple<double, double, double> spread_percentiles(int64_t start_time,
                                                        int64_t end_time) const;

  std::tuple<double, double, double>
  spread_percentiles_exact(int64_t start_time, int64_t end_time) const;

  double min_spread(int64_t start_time, int64_t end_time) const;
  double max_spread(int64_t start_time, int64_t end_time) const;

  // --- Accessors ---
  double hist_min() const { return hist_min_; }
  double hist_max() const {
    return hist_min_ + NUM_HIST_BINS * hist_bin_width_;
  }
  double hist_bin_width() const { return hist_bin_width_; }

private:
  static constexpr double POS_INF = std::numeric_limits<double>::infinity();
  static constexpr double NEG_INF = -std::numeric_limits<double>::infinity();

  // ---- Histogram helpers ----
  double hist_min_;
  double hist_bin_width_;

  int spread_to_bin(double spread) const {
    int b = static_cast<int>((spread - hist_min_) / hist_bin_width_);
    if (b < 0)
      b = 0;
    if (b >= NUM_HIST_BINS)
      b = NUM_HIST_BINS - 1;
    return b;
  }
  double bin_mid(int b) const {
    return hist_min_ + (b + 0.5) * hist_bin_width_;
  }

  // ---- Bucket ----
  struct Bucket {
    int64_t abs_index = -1;
    int64_t entry_count = 0;
    double min_spread = POS_INF;
    double max_spread = NEG_INF;
    std::vector<double> spreads;
    std::array<int32_t, NUM_HIST_BINS> hist{};

    void clear() {
      abs_index = -1;
      entry_count = 0;
      min_spread = POS_INF;
      max_spread = NEG_INF;
      spreads.clear();
      hist.fill(0);
    }
  };

  std::vector<Bucket> buckets_;

  // ---- Segment tree (count / min / max) ----
  struct SegNode {
    int64_t count = 0;
    double min_spread = POS_INF;
    double max_spread = NEG_INF;
  };
  std::vector<SegNode> seg_;

  void seg_update(int node, int lo, int hi, int pos);
  SegNode seg_query(int node, int lo, int hi, int ql, int qr) const;

  // ---- Fenwick trees (one per histogram bin) ----
  std::vector<int32_t> fenwick_;

  int fw_idx(int bin, int pos) const { return bin * (NUM_BUCKETS + 1) + pos; }
  void fw_update(int bin, int local_pos, int delta);
  int fw_prefix(int bin, int local_pos) const;
  int fw_range(int bin, int l, int r) const;

  // ---- Window bookkeeping ----
  int64_t total_count_ = 0;
  int64_t window_start_abs_ = INT64_MAX;
  int64_t window_end_abs_ = INT64_MIN;

  static int64_t to_abs_bucket(int64_t time_ns);
  static int to_local(int64_t abs_bucket);

  void clear_bucket_at(int local);

  SegNode query_seg_range(int64_t start_abs, int64_t end_abs) const;
  int query_fw_range(int bin, int64_t start_abs, int64_t end_abs) const;

  void collect_spreads(int64_t start_abs, int64_t end_abs,
                       std::vector<double> &out) const;

  // Thread safety — mutable so const query methods can lock.
  mutable std::shared_mutex mutex_;
};