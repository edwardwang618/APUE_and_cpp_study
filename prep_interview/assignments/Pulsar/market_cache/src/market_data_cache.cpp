// =============================================================================
// market_data_cache.cpp — implementation
// =============================================================================
#include "market_data_cache.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <mutex>
#include <sstream>
#include <stdexcept>

// =============================================================================
// MarketDataEntry
// =============================================================================

double MarketDataEntry::compute_spread() const {
  if (bids.empty() || asks.empty())
    return std::numeric_limits<double>::quiet_NaN();

  double highest_bid = -std::numeric_limits<double>::infinity();
  for (auto &b : bids)
    if (b.price > highest_bid)
      highest_bid = b.price;

  double lowest_ask = std::numeric_limits<double>::infinity();
  for (auto &a : asks)
    if (a.price < lowest_ask)
      lowest_ask = a.price;

  return lowest_ask - highest_bid;
}

// =============================================================================
// Minimal JSON helpers  (handles the specific market_data.json format)
// =============================================================================
namespace {

void skip_ws(const std::string &s, size_t &i) {
  while (i < s.size() &&
         (s[i] == ' ' || s[i] == '\n' || s[i] == '\r' || s[i] == '\t'))
    ++i;
}

void expect(const std::string &s, size_t &i, char c) {
  skip_ws(s, i);
  if (i >= s.size() || s[i] != c)
    throw std::runtime_error(std::string("JSON parse: expected '") + c +
                             "' at pos " + std::to_string(i));
  ++i;
}

std::string parse_string(const std::string &s, size_t &i) {
  skip_ws(s, i);
  if (i >= s.size() || s[i] != '"')
    throw std::runtime_error("JSON parse: expected '\"'");
  ++i;
  std::string result;
  while (i < s.size() && s[i] != '"') {
    if (s[i] == '\\') {
      ++i;
      if (i < s.size())
        result += s[i++];
    } else
      result += s[i++];
  }
  if (i < s.size())
    ++i;
  return result;
}

double parse_number(const std::string &s, size_t &i) {
  skip_ws(s, i);
  size_t start = i;
  if (i < s.size() && s[i] == '-')
    ++i;
  while (i < s.size() && (s[i] >= '0' && s[i] <= '9'))
    ++i;
  if (i < s.size() && s[i] == '.') {
    ++i;
    while (i < s.size() && (s[i] >= '0' && s[i] <= '9'))
      ++i;
  }
  if (i < s.size() && (s[i] == 'e' || s[i] == 'E')) {
    ++i;
    if (i < s.size() && (s[i] == '+' || s[i] == '-'))
      ++i;
    while (i < s.size() && (s[i] >= '0' && s[i] <= '9'))
      ++i;
  }
  return std::stod(s.substr(start, i - start));
}

int64_t parse_int64(const std::string &s, size_t &i) {
  skip_ws(s, i);
  size_t start = i;
  if (i < s.size() && s[i] == '-')
    ++i;
  while (i < s.size() && (s[i] >= '0' && s[i] <= '9'))
    ++i;
  // Use strtoull to handle large unsigned values that fit in int64_t bits
  return static_cast<int64_t>(
      std::strtoull(s.substr(start, i - start).c_str(), nullptr, 10));
}

PriceLevel parse_price_level(const std::string &s, size_t &i) {
  PriceLevel pl{};
  expect(s, i, '{');
  for (int field = 0; field < 2; ++field) {
    if (field > 0)
      expect(s, i, ',');
    std::string key = parse_string(s, i);
    expect(s, i, ':');
    double val = parse_number(s, i);
    if (key == "price")
      pl.price = val;
    else if (key == "amount")
      pl.amount = val;
  }
  expect(s, i, '}');
  return pl;
}

std::vector<PriceLevel> parse_price_level_array(const std::string &s,
                                                size_t &i) {
  std::vector<PriceLevel> result;
  expect(s, i, '[');
  skip_ws(s, i);
  if (i < s.size() && s[i] == ']') {
    ++i;
    return result;
  }
  result.push_back(parse_price_level(s, i));
  while (true) {
    skip_ws(s, i);
    if (i >= s.size() || s[i] != ',')
      break;
    ++i;
    result.push_back(parse_price_level(s, i));
  }
  expect(s, i, ']');
  return result;
}

MarketDataEntry parse_entry(const std::string &s, size_t &i) {
  MarketDataEntry entry{};
  expect(s, i, '{');
  bool first = true;
  while (true) {
    skip_ws(s, i);
    if (i >= s.size() || s[i] == '}')
      break;
    if (!first)
      expect(s, i, ',');
    first = false;
    std::string key = parse_string(s, i);
    expect(s, i, ':');
    if (key == "utc_epoch_ns") {
      entry.time = parse_int64(s, i);
    } else if (key == "bids") {
      entry.bids = parse_price_level_array(s, i);
    } else if (key == "asks") {
      entry.asks = parse_price_level_array(s, i);
    }
  }
  expect(s, i, '}');
  return entry;
}

std::vector<MarketDataEntry> parse_market_data_json(const std::string &json) {
  std::vector<MarketDataEntry> entries;
  size_t i = 0;
  expect(json, i, '{');
  parse_string(json, i);
  expect(json, i, ':');
  expect(json, i, '[');
  skip_ws(json, i);
  if (i < json.size() && json[i] == ']') {
    i++;
    expect(json, i, '}');
    return entries;
  }
  entries.push_back(parse_entry(json, i));
  while (true) {
    skip_ws(json, i);
    if (i >= json.size() || json[i] != ',')
      break;
    ++i;
    entries.push_back(parse_entry(json, i));
  }
  expect(json, i, ']');
  return entries;
}

} // anonymous namespace

// =============================================================================
// Construction
// =============================================================================

MarketDataCache::MarketDataCache(double hist_min, double hist_max)
    : hist_min_(hist_min),
      hist_bin_width_((hist_max - hist_min) / NUM_HIST_BINS),
      buckets_(NUM_BUCKETS), seg_(SEG_TREE_SIZE),
      fenwick_(NUM_HIST_BINS * (NUM_BUCKETS + 1), 0) {
  if (hist_max <= hist_min || hist_bin_width_ <= 0)
    throw std::invalid_argument("Invalid histogram range");
}

// Move constructor: move all data members, construct a fresh mutex.
MarketDataCache::MarketDataCache(MarketDataCache &&other) noexcept
    : hist_min_(other.hist_min_), hist_bin_width_(other.hist_bin_width_),
      buckets_(std::move(other.buckets_)), seg_(std::move(other.seg_)),
      fenwick_(std::move(other.fenwick_)), total_count_(other.total_count_),
      window_start_abs_(other.window_start_abs_),
      window_end_abs_(other.window_end_abs_)
// mutex_ is default-constructed (fresh mutex)
{
  other.total_count_ = 0;
  other.window_start_abs_ = INT64_MAX;
  other.window_end_abs_ = INT64_MIN;
}

MarketDataCache &MarketDataCache::operator=(MarketDataCache &&other) noexcept {
  if (this != &other) {
    hist_min_ = other.hist_min_;
    hist_bin_width_ = other.hist_bin_width_;
    buckets_ = std::move(other.buckets_);
    seg_ = std::move(other.seg_);
    fenwick_ = std::move(other.fenwick_);
    total_count_ = other.total_count_;
    window_start_abs_ = other.window_start_abs_;
    window_end_abs_ = other.window_end_abs_;
    // mutex_ stays as-is (already constructed)

    other.total_count_ = 0;
    other.window_start_abs_ = INT64_MAX;
    other.window_end_abs_ = INT64_MIN;
  }
  return *this;
}

MarketDataCache MarketDataCache::with_file(const std::string &file_path,
                                           double hist_min, double hist_max) {
  std::ifstream ifs(file_path);
  if (!ifs)
    throw std::runtime_error("Cannot open file: " + file_path);
  std::ostringstream oss;
  oss << ifs.rdbuf();
  std::string json = oss.str();

  auto entries = parse_market_data_json(json);

  MarketDataCache cache(hist_min, hist_max);
  for (auto &e : entries)
    cache.insert(e);
  return cache; // uses move constructor
}

// =============================================================================
// Helpers
// =============================================================================

int64_t MarketDataCache::to_abs_bucket(int64_t time_ns) {
  if (time_ns >= 0)
    return time_ns / BUCKET_NS;
  return (time_ns - BUCKET_NS + 1) / BUCKET_NS;
}

int MarketDataCache::to_local(int64_t abs_bucket) {
  int local = static_cast<int>(abs_bucket % NUM_BUCKETS);
  if (local < 0)
    local += NUM_BUCKETS;
  return local;
}

void MarketDataCache::clear_bucket_at(int local) {
  Bucket &b = buckets_[local];
  if (b.entry_count == 0)
    return;

  total_count_ -= b.entry_count;

  for (int bin = 0; bin < NUM_HIST_BINS; ++bin) {
    if (b.hist[bin] != 0)
      fw_update(bin, local, -b.hist[bin]);
  }

  b.clear();
  seg_update(1, 0, NUM_BUCKETS - 1, local);
}

// =============================================================================
// Segment tree
// =============================================================================

void MarketDataCache::seg_update(int node, int lo, int hi, int pos) {
  if (lo == hi) {
    auto &b = buckets_[pos];
    seg_[node].count = b.entry_count;
    seg_[node].min_spread = b.min_spread;
    seg_[node].max_spread = b.max_spread;
    return;
  }
  int mid = (lo + hi) / 2;
  if (pos <= mid)
    seg_update(2 * node, lo, mid, pos);
  else
    seg_update(2 * node + 1, mid + 1, hi, pos);
  seg_[node].count = seg_[2 * node].count + seg_[2 * node + 1].count;
  seg_[node].min_spread =
      std::min(seg_[2 * node].min_spread, seg_[2 * node + 1].min_spread);
  seg_[node].max_spread =
      std::max(seg_[2 * node].max_spread, seg_[2 * node + 1].max_spread);
}

MarketDataCache::SegNode MarketDataCache::seg_query(int node, int lo, int hi,
                                                    int ql, int qr) const {
  if (qr < lo || hi < ql)
    return {0, POS_INF, NEG_INF};
  if (ql <= lo && hi <= qr)
    return seg_[node];
  int mid = (lo + hi) / 2;
  auto L = seg_query(2 * node, lo, mid, ql, qr);
  auto R = seg_query(2 * node + 1, mid + 1, hi, ql, qr);
  return {L.count + R.count, std::min(L.min_spread, R.min_spread),
          std::max(L.max_spread, R.max_spread)};
}

// =============================================================================
// Fenwick trees
// =============================================================================

void MarketDataCache::fw_update(int bin, int local_pos, int delta) {
  for (int p = local_pos + 1; p <= NUM_BUCKETS; p += p & (-p))
    fenwick_[fw_idx(bin, p)] += delta;
}

int MarketDataCache::fw_prefix(int bin, int local_pos) const {
  int sum = 0;
  for (int p = local_pos + 1; p > 0; p -= p & (-p))
    sum += fenwick_[fw_idx(bin, p)];
  return sum;
}

int MarketDataCache::fw_range(int bin, int l, int r) const {
  if (l > r)
    return 0;
  return fw_prefix(bin, r) - (l > 0 ? fw_prefix(bin, l - 1) : 0);
}

// =============================================================================
// Wraparound-aware range helpers
// =============================================================================

MarketDataCache::SegNode
MarketDataCache::query_seg_range(int64_t start_abs, int64_t end_abs) const {
  if (window_start_abs_ > window_end_abs_)
    return {0, POS_INF, NEG_INF};

  start_abs = std::max(start_abs, window_start_abs_);
  end_abs = std::min(end_abs, window_end_abs_);
  if (start_abs > end_abs)
    return {0, POS_INF, NEG_INF};

  int sl = to_local(start_abs);
  int el = to_local(end_abs);

  int64_t span = end_abs - start_abs + 1;
  if (span >= NUM_BUCKETS)
    return seg_query(1, 0, NUM_BUCKETS - 1, 0, NUM_BUCKETS - 1);

  if (sl <= el) {
    return seg_query(1, 0, NUM_BUCKETS - 1, sl, el);
  } else {
    auto a = seg_query(1, 0, NUM_BUCKETS - 1, sl, NUM_BUCKETS - 1);
    auto b = seg_query(1, 0, NUM_BUCKETS - 1, 0, el);
    return {a.count + b.count, std::min(a.min_spread, b.min_spread),
            std::max(a.max_spread, b.max_spread)};
  }
}

int MarketDataCache::query_fw_range(int bin, int64_t start_abs,
                                    int64_t end_abs) const {
  if (window_start_abs_ > window_end_abs_)
    return 0;

  start_abs = std::max(start_abs, window_start_abs_);
  end_abs = std::min(end_abs, window_end_abs_);
  if (start_abs > end_abs)
    return 0;

  int sl = to_local(start_abs);
  int el = to_local(end_abs);

  int64_t span = end_abs - start_abs + 1;
  if (span >= NUM_BUCKETS)
    return fw_range(bin, 0, NUM_BUCKETS - 1);

  if (sl <= el) {
    return fw_range(bin, sl, el);
  } else {
    return fw_range(bin, sl, NUM_BUCKETS - 1) + fw_range(bin, 0, el);
  }
}

void MarketDataCache::collect_spreads(int64_t start_abs, int64_t end_abs,
                                      std::vector<double> &out) const {
  if (window_start_abs_ > window_end_abs_)
    return;

  start_abs = std::max(start_abs, window_start_abs_);
  end_abs = std::min(end_abs, window_end_abs_);
  if (start_abs > end_abs)
    return;

  for (int64_t ab = start_abs; ab <= end_abs; ++ab) {
    int local = to_local(ab);
    auto &bkt = buckets_[local];
    if (bkt.abs_index == ab && bkt.entry_count > 0) {
      out.insert(out.end(), bkt.spreads.begin(), bkt.spreads.end());
    }
  }
}

// =============================================================================
// insert
// =============================================================================

void MarketDataCache::insert(const MarketDataEntry &data) {
  double spread = data.compute_spread();
  if (std::isnan(spread))
    return;

  int64_t abs = to_abs_bucket(data.time);

  std::unique_lock<std::shared_mutex> lock(mutex_);

  // --- Window management ---
  if (window_start_abs_ > window_end_abs_) {
    window_start_abs_ = abs;
    window_end_abs_ = abs;
  } else if (abs > window_end_abs_) {
    int64_t new_start = abs - NUM_BUCKETS + 1;
    if (new_start > window_start_abs_) {
      int64_t evict_count = new_start - window_start_abs_;
      if (evict_count >= NUM_BUCKETS) {
        for (int i = 0; i < NUM_BUCKETS; ++i)
          clear_bucket_at(i);
      } else {
        for (int64_t b = window_start_abs_; b < new_start; ++b) {
          int local = to_local(b);
          if (buckets_[local].abs_index == b)
            clear_bucket_at(local);
        }
      }
      window_start_abs_ = new_start;
    }
    window_end_abs_ = abs;
  } else if (abs < window_start_abs_) {
    return;
  }

  // --- Insert into bucket ---
  int local = to_local(abs);
  Bucket &bkt = buckets_[local];

  if (bkt.abs_index != -1 && bkt.abs_index != abs) {
    clear_bucket_at(local);
  }
  if (bkt.abs_index == -1) {
    bkt.abs_index = abs;
  }

  bkt.entry_count++;
  if (spread < bkt.min_spread)
    bkt.min_spread = spread;
  if (spread > bkt.max_spread)
    bkt.max_spread = spread;
  bkt.spreads.push_back(spread);

  int bin = spread_to_bin(spread);
  bkt.hist[bin]++;

  total_count_++;

  seg_update(1, 0, NUM_BUCKETS - 1, local);
  fw_update(bin, local, 1);
}

// =============================================================================
// remove_up_to
// =============================================================================

void MarketDataCache::remove_up_to(int64_t time) {
  int64_t abs_limit = to_abs_bucket(time);

  std::unique_lock<std::shared_mutex> lock(mutex_);

  if (window_start_abs_ > window_end_abs_)
    return;

  int64_t new_start = abs_limit + 1;
  if (new_start <= window_start_abs_)
    return;

  if (new_start > window_end_abs_) {
    for (int i = 0; i < NUM_BUCKETS; ++i)
      clear_bucket_at(i);
    window_start_abs_ = INT64_MAX;
    window_end_abs_ = INT64_MIN;
    return;
  }

  int64_t evict_count = new_start - window_start_abs_;
  if (evict_count >= NUM_BUCKETS) {
    for (int i = 0; i < NUM_BUCKETS; ++i)
      clear_bucket_at(i);
  } else {
    for (int64_t b = window_start_abs_; b < new_start; ++b) {
      int local = to_local(b);
      if (buckets_[local].abs_index == b)
        clear_bucket_at(local);
    }
  }
  window_start_abs_ = new_start;
}

// =============================================================================
// count / count_range
// =============================================================================

int64_t MarketDataCache::count() const {
  std::shared_lock<std::shared_mutex> lock(mutex_);
  return total_count_;
}

int64_t MarketDataCache::count_range(int64_t start_time,
                                     int64_t end_time) const {
  int64_t sa = to_abs_bucket(start_time);
  int64_t ea = to_abs_bucket(end_time);

  std::shared_lock<std::shared_mutex> lock(mutex_);
  return query_seg_range(sa, ea).count;
}

// =============================================================================
// min_spread / max_spread
// =============================================================================

double MarketDataCache::min_spread(int64_t start_time, int64_t end_time) const {
  int64_t sa = to_abs_bucket(start_time);
  int64_t ea = to_abs_bucket(end_time);

  std::shared_lock<std::shared_mutex> lock(mutex_);
  auto res = query_seg_range(sa, ea);
  return (res.count > 0) ? res.min_spread
                         : std::numeric_limits<double>::quiet_NaN();
}

double MarketDataCache::max_spread(int64_t start_time, int64_t end_time) const {
  int64_t sa = to_abs_bucket(start_time);
  int64_t ea = to_abs_bucket(end_time);

  std::shared_lock<std::shared_mutex> lock(mutex_);
  auto res = query_seg_range(sa, ea);
  return (res.count > 0) ? res.max_spread
                         : std::numeric_limits<double>::quiet_NaN();
}

// =============================================================================
// spread_percentiles  (approximate, histogram-based)
// =============================================================================

std::tuple<double, double, double>
MarketDataCache::spread_percentiles(int64_t start_time,
                                    int64_t end_time) const {
  int64_t sa = to_abs_bucket(start_time);
  int64_t ea = to_abs_bucket(end_time);

  std::shared_lock<std::shared_mutex> lock(mutex_);

  auto seg_res = query_seg_range(sa, ea);
  int64_t total = seg_res.count;
  if (total == 0) {
    double nan = std::numeric_limits<double>::quiet_NaN();
    return {nan, nan, nan};
  }

  int64_t r10 =
      std::max<int64_t>(1, static_cast<int64_t>(std::ceil(0.10 * total)));
  int64_t r50 =
      std::max<int64_t>(1, static_cast<int64_t>(std::ceil(0.50 * total)));
  int64_t r90 =
      std::max<int64_t>(1, static_cast<int64_t>(std::ceil(0.90 * total)));

  double p10 = std::numeric_limits<double>::quiet_NaN();
  double p50 = std::numeric_limits<double>::quiet_NaN();
  double p90 = std::numeric_limits<double>::quiet_NaN();

  int64_t cumulative = 0;
  for (int bin = 0; bin < NUM_HIST_BINS; ++bin) {
    int cnt = query_fw_range(bin, sa, ea);
    if (cnt == 0)
      continue;
    cumulative += cnt;

    double val = bin_mid(bin);
    if (std::isnan(p10) && cumulative >= r10)
      p10 = val;
    if (std::isnan(p50) && cumulative >= r50)
      p50 = val;
    if (std::isnan(p90) && cumulative >= r90)
      p90 = val;

    if (!std::isnan(p90))
      break;
  }

  return {p10, p50, p90};
}

// =============================================================================
// spread_percentiles_exact
// =============================================================================

std::tuple<double, double, double>
MarketDataCache::spread_percentiles_exact(int64_t start_time,
                                          int64_t end_time) const {
  int64_t sa = to_abs_bucket(start_time);
  int64_t ea = to_abs_bucket(end_time);

  std::shared_lock<std::shared_mutex> lock(mutex_);

  std::vector<double> spreads;
  collect_spreads(sa, ea, spreads);

  size_t N = spreads.size();
  if (N == 0) {
    double nan = std::numeric_limits<double>::quiet_NaN();
    return {nan, nan, nan};
  }

  auto rank_idx = [&](double p) -> size_t {
    size_t r = static_cast<size_t>(std::ceil(p * N));
    if (r == 0)
      r = 1;
    return r - 1;
  };
  size_t i10 = rank_idx(0.10);
  size_t i50 = rank_idx(0.50);
  size_t i90 = rank_idx(0.90);

  std::nth_element(spreads.begin(), spreads.begin() + static_cast<long>(i10),
                   spreads.end());
  double p10 = spreads[i10];

  std::nth_element(spreads.begin() + static_cast<long>(i10),
                   spreads.begin() + static_cast<long>(i50), spreads.end());
  double p50 = spreads[i50];

  std::nth_element(spreads.begin() + static_cast<long>(i50),
                   spreads.begin() + static_cast<long>(i90), spreads.end());
  double p90 = spreads[i90];

  return {p10, p50, p90};
}