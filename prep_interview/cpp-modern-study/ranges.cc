#include <algorithm>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

struct Trade {
  std::string symbol;
  double price;
  int qty;
};

int main() {
  std::vector<Trade> trades = {
      {"AAPL", 150.0, 100}, {"GOOG", 2800.0, 10}, {"AAPL", 151.0, 50},
      {"MSFT", 300.0, 75},  {"AAPL", 149.5, 200},
  };

  auto aapl_notional =
      trades |
      std::views::filter([](const Trade &t) { return t.symbol == "AAPL"; }) |
      std::views::transform([](const Trade &t) { return t.price * t.qty; });

  double total = 0;
  for (double n : aapl_notional)
    total += n;
  std::cout << "AAPL notional: " << total << '\n';

  std::vector<int> v = {3, 1, 4, 1, 5, 9, 2, 6};

  namespace rg = std::ranges;
  rg::sort(v); // no v.begin(), v.end()
  auto it = rg::find(v, 5);
  std::cout << (it != v.end()) << std::endl;
  bool any = rg::any_of(v, [](int x) { return x > 5; });
  auto cnt = rg::count_if(v, [](int x) { return x % 2 == 0; });
}