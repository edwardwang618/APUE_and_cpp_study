#include <atomic>
#include <iostream>

int main() {

  // ============================================================
  // exchange
  // ============================================================

  std::atomic<int> x{10};

  int old = x.exchange(20);

  std::cout << "exchange:\n";
  std::cout << "  old = " << old << "\n"; // 10
  std::cout << "  x   = " << x << "\n\n"; // 20

  // ============================================================
  // compare_exchange_strong (success)
  // ============================================================

  std::atomic<int> y{10};
  int expected = 10;

  bool ok = y.compare_exchange_strong(expected, 20);

  std::cout << "compare_exchange_strong (success):\n";
  std::cout << "  ok       = " << ok << "\n";       // true
  std::cout << "  expected = " << expected << "\n"; // 10 (unchanged)
  std::cout << "  y        = " << y << "\n\n";      // 20

  // ============================================================
  // compare_exchange_strong (failure)
  // ============================================================

  std::atomic<int> z{99};
  expected = 10;

  ok = z.compare_exchange_strong(expected, 20);

  std::cout << "compare_exchange_strong (failure):\n";
  std::cout << "  ok       = " << ok << "\n";       // false
  std::cout << "  expected = " << expected << "\n"; // 99 (updated!)
  std::cout << "  z        = " << z << "\n\n";      // 99 (unchanged)

  // ============================================================
  // Typical loop: atomic increment
  // ============================================================

  std::atomic<int> counter{5};

  int exp = counter.load();
  while (!counter.compare_exchange_weak(exp, exp + 1)) {
    // exp auto-updated on failure, retry
  }

  std::cout << "atomic increment:\n";
  std::cout << "  counter = " << counter << "\n"; // 6

  return 0;
}