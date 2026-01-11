#include <algorithm>
#include <functional>
#include <iostream>
#include <set>

int main() {

  //===========================================
  // 1. BASIC CONSTRUCTION
  //===========================================

  std::set<int> s1;                                // Empty set
  std::set<int> s2 = {5, 2, 8, 1, 9};              // Initializer list
  std::set<int> s3(s2);                            // Copy constructor
  std::set<int> s4(s2.begin(), s2.end());          // Range constructor
  std::set<int, std::greater<int>> s5 = {5, 2, 8}; // Descending order

  std::cout << "s5 (descending): ";
  for (int x : s5)
    std::cout << x << " "; // 8 5 2
  std::cout << "\n\n";

  //===========================================
  // 2. INSERTION
  //===========================================

  std::set<int> s;

  // Single insert - returns pair<iterator, bool>
  auto [it1, inserted1] = s.insert(10);
  std::cout << "Inserted 10: " << inserted1 << "\n"; // 1 (true)

  auto [it2, inserted2] = s.insert(10);                    // Duplicate
  std::cout << "Inserted 10 again: " << inserted2 << "\n"; // 0 (false)

  // Insert with hint (O(1) if hint is correct position)
  // Useful in HFT when you know approximate position
  auto hint = s.end();
  s.insert(hint, 20); // O(1) if 20 goes at end

  // Range insert
  std::vector<int> v = {3, 6, 9};
  s.insert(v.begin(), v.end());

  // Initializer list insert
  s.insert({1, 2, 4, 5});

  // emplace - constructs in place, avoids copy
  s.emplace(15);

  // emplace_hint
  s.emplace_hint(s.end(), 25);

  std::cout << "Set contents: ";
  for (int x : s)
    std::cout << x << " ";
  std::cout << "\n\n";

  //===========================================
  // 3. LOOKUP - CRITICAL FOR HFT
  //===========================================

  // find() - O(log n)
  auto it = s.find(10);
  if (it != s.end()) {
    std::cout << "Found: " << *it << "\n";
  }

  // count() - returns 0 or 1 for set
  std::cout << "Count of 10: " << s.count(10) << "\n";
  std::cout << "Count of 100: " << s.count(100) << "\n";

  // contains() - C++20, cleaner than find
  // if (s.contains(10)) { ... }

  // lower_bound - first element >= value
  auto lb = s.lower_bound(7);
  std::cout << "lower_bound(7): " << *lb << "\n"; // 9

  // upper_bound - first element > value
  auto ub = s.upper_bound(7);
  std::cout << "upper_bound(7): " << *ub << "\n"; // 9

  auto lb2 = s.lower_bound(6);
  std::cout << "lower_bound(6): " << *lb2 << "\n"; // 6

  auto ub2 = s.upper_bound(6);
  std::cout << "upper_bound(6): " << *ub2 << "\n"; // 9

  // equal_range - returns pair of lower_bound and upper_bound
  auto [lo, hi] = s.equal_range(6);
  std::cout << "equal_range(6): [" << *lo << ", " << *hi << ")\n\n";

  //===========================================
  // 4. DELETION
  //===========================================

  // Erase by value
  size_t erased = s.erase(10);
  std::cout << "Erased 10, count: " << erased << "\n";

  // Erase by iterator
  auto it_to_erase = s.find(15);
  if (it_to_erase != s.end()) {
    s.erase(it_to_erase);
  }

  // Erase range
  auto start = s.find(3);
  auto end = s.find(6);
  s.erase(start, end); // Erases [3, 6), so 3, 4, 5

  // clear()
  // s.clear();

  std::cout << "After deletions: ";
  for (int x : s)
    std::cout << x << " ";
  std::cout << "\n\n";

  //===========================================
  // 5. CAPACITY
  //===========================================

  std::cout << "Size: " << s.size() << "\n";
  std::cout << "Empty: " << s.empty() << "\n";
  std::cout << "Max size: " << s.max_size() << "\n\n";

  //===========================================
  // 6. ITERATORS
  //===========================================

  s = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  // Forward iteration
  std::cout << "Forward: ";
  for (auto it = s.begin(); it != s.end(); ++it) {
    std::cout << *it << " ";
  }
  std::cout << "\n";

  // Reverse iteration
  std::cout << "Reverse: ";
  for (auto it = s.rbegin(); it != s.rend(); ++it) {
    std::cout << *it << " ";
  }
  std::cout << "\n\n";

  //===========================================
  // 7. HFT USE CASE: ORDER BOOK PRICE LEVELS
  //===========================================

  std::cout << "=== HFT ORDER BOOK EXAMPLE ===\n\n";

  // Bid prices - descending (highest first)
  std::set<double, std::greater<double>> bid_prices;
  bid_prices.insert({100.50, 100.45, 100.40, 100.35});

  // Ask prices - ascending (lowest first)
  std::set<double> ask_prices;
  ask_prices.insert({100.55, 100.60, 100.65, 100.70});

  std::cout << "Bid prices (best first): ";
  for (double p : bid_prices)
    std::cout << p << " ";
  std::cout << "\n";

  std::cout << "Ask prices (best first): ";
  for (double p : ask_prices)
    std::cout << p << " ";
  std::cout << "\n";

  // Best bid/ask
  double best_bid = *bid_prices.begin();
  double best_ask = *ask_prices.begin();
  double spread = best_ask - best_bid;

  std::cout << "Best bid: " << best_bid << "\n";
  std::cout << "Best ask: " << best_ask << "\n";
  std::cout << "Spread: " << spread << "\n\n";

  // Find price levels within range
  auto low = ask_prices.lower_bound(100.58);
  auto high = ask_prices.upper_bound(100.68);

  std::cout << "Ask prices in range [100.58, 100.68]: ";
  for (auto it = low; it != high; ++it) {
    std::cout << *it << " ";
  }
  std::cout << "\n\n";

  //===========================================
  // 8. HFT USE CASE: TRACKING ACTIVE ORDER IDS
  //===========================================

  std::cout << "=== ACTIVE ORDER TRACKING ===\n\n";

  std::set<uint64_t> active_orders;

  // New orders arrive
  active_orders.insert(1001);
  active_orders.insert(1002);
  active_orders.insert(1003);
  active_orders.insert(1005);

  // Check if order exists - O(log n)
  uint64_t order_to_cancel = 1002;
  if (active_orders.count(order_to_cancel)) {
    std::cout << "Order " << order_to_cancel << " found, cancelling\n";
    active_orders.erase(order_to_cancel);
  }

  // Check for missing order
  if (!active_orders.count(9999)) {
    std::cout << "Order 9999 not found\n";
  }

  std::cout << "Active orders: ";
  for (auto id : active_orders)
    std::cout << id << " ";
  std::cout << "\n\n";

  //===========================================
  // 9. HFT USE CASE: TIME-PRIORITY QUEUE
  //===========================================

  std::cout << "=== TIME-PRIORITY ORDER QUEUE ===\n\n";

  struct Order {
    uint64_t timestamp;
    uint64_t order_id;
    double price;
    int quantity;

    // Compare by timestamp for time priority
    bool operator<(const Order &other) const {
      return timestamp < other.timestamp;
    }
  };

  std::set<Order> order_queue;

  order_queue.insert({1000, 101, 100.50, 100});
  order_queue.insert({1001, 102, 100.50, 200});
  order_queue.insert({999, 103, 100.50, 50}); // Earlier timestamp

  std::cout << "Orders by time priority:\n";
  for (const auto &order : order_queue) {
    std::cout << "  ts=" << order.timestamp << " id=" << order.order_id
              << " qty=" << order.quantity << "\n";
  }
  std::cout << "\n";

  //===========================================
  // 10. EXTRACT AND MERGE (C++17)
  //===========================================

  std::cout << "=== EXTRACT AND MERGE (C++17) ===\n\n";

  std::set<int> set1 = {1, 2, 3};
  std::set<int> set2 = {4, 5, 6};

  // Extract node without deallocation
  auto node = set1.extract(2);
  if (!node.empty()) {
    node.value() = 20; // Modify extracted value
    set2.insert(std::move(node));
  }

  std::cout << "set1: ";
  for (int x : set1)
    std::cout << x << " "; // 1 3
  std::cout << "\n";

  std::cout << "set2: ";
  for (int x : set2)
    std::cout << x << " "; // 4 5 6 20
  std::cout << "\n";

  // Merge - moves elements from source to destination
  set1.merge(set2);

  std::cout << "After merge, set1: ";
  for (int x : set1)
    std::cout << x << " ";
  std::cout << "\n\n";

  //===========================================
  // 11. CUSTOM COMPARATOR FOR COMPLEX KEYS
  //===========================================

  std::cout << "=== CUSTOM COMPARATOR ===\n\n";

  // Lambda comparator (C++20 makes this easier)
  auto price_level_cmp = [](const std::pair<double, int> &a,
                            const std::pair<double, int> &b) {
    return a.first < b.first; // Compare by price only
  };

  std::set<std::pair<double, int>, decltype(price_level_cmp)> price_levels(
      price_level_cmp);

  price_levels.insert({100.50, 1000});
  price_levels.insert({100.55, 500});
  price_levels.insert({100.45, 750});

  std::cout << "Price levels:\n";
  for (const auto &[price, qty] : price_levels) {
    std::cout << "  " << price << " -> " << qty << "\n";
  }
  std::cout << "\n";

  //===========================================
  // 12. PERFORMANCE COMPARISON
  //===========================================

  std::cout << "=== PERFORMANCE NOTES ===\n\n";

  std::cout << "std::set operations:\n";
  std::cout << "  insert:      O(log n)\n";
  std::cout << "  erase:       O(log n)\n";
  std::cout << "  find:        O(log n)\n";
  std::cout << "  lower_bound: O(log n)\n";
  std::cout << "  iteration:   O(n)\n\n";

  std::cout << "HFT considerations:\n";
  std::cout << "  - Use unordered_set for O(1) lookup if order not needed\n";
  std::cout << "  - Use set when you need sorted iteration or range queries\n";
  std::cout << "  - insert with hint is O(1) if position is correct\n";
  std::cout << "  - extract() avoids reallocation when moving nodes\n";
  std::cout << "  - Consider flat_set (C++23) for cache-friendly iteration\n";

  return 0;
}