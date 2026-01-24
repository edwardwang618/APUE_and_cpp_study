#include <chrono>
#include <iostream>
#include <random>
#include <set>

struct Order {
  uint64_t id;
  double price;
  int quantity;
  char padding[64]; // Make it bigger

  bool operator<(const Order &o) const {
    return price < o.price || (price == o.price && id < o.id);
  }
};

int main() {
  constexpr int N = 100000;
  constexpr int MODIFICATIONS = 50000;

  std::mt19937 rng(42);

  //===========================================
  // Setup
  //===========================================
  std::set<Order> s1, s2;
  std::vector<double> prices;

  for (int i = 0; i < N; ++i) {
    double price = (rng() % 10000) / 100.0;
    prices.push_back(price);
    s1.insert({(uint64_t)i, price, 100, {}});
    s2.insert({(uint64_t)i, price, 100, {}});
  }

  //===========================================
  // Old way: erase + insert
  //===========================================
  auto start1 = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < MODIFICATIONS; ++i) {
    int idx = rng() % N;
    Order target{(uint64_t)idx, prices[idx], 0, {}};
    auto it = s1.find(target);

    if (it != s1.end()) {
      Order modified = *it;
      modified.quantity = rng() % 1000;
      // Note: NOT changing price (key), just quantity
      s1.erase(it);
      s1.insert(modified);
    }
  }

  auto end1 = std::chrono::high_resolution_clock::now();
  auto time1 =
      std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1);

  //===========================================
  // Extract way
  //===========================================
  auto start2 = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < MODIFICATIONS; ++i) {
    int idx = rng() % N;
    Order target{(uint64_t)idx, prices[idx], 0, {}};
    auto it = s2.find(target);

    if (it != s2.end()) {
      auto node = s2.extract(it);
      node.value().quantity = rng() % 1000;
      s2.insert(std::move(node));
    }
  }

  auto end2 = std::chrono::high_resolution_clock::now();
  auto time2 =
      std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2);

  //===========================================
  // Results
  //===========================================
  std::cout << "Old way (erase+insert): " << time1.count() << " us\n";
  std::cout << "Extract way:            " << time2.count() << " us\n";
  std::cout << "Speedup:                "
            << (double)time1.count() / time2.count() << "x\n";
}