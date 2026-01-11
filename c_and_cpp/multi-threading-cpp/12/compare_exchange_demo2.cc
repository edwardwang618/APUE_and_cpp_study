#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

std::vector<int> data;
std::atomic<int> flag{0};

int main() {
  for (int i = 0; i < 1000000000; i++) {
    std::thread t1([&]() {
      data.push_back(42);
      flag.store(1, std::memory_order_release);
    });

    std::thread t2([&]() {
      int exp = 1;
      while (!flag.compare_exchange_strong(exp, 2, std::memory_order_relaxed))
        exp = 1;
    });

    std::thread t3([&]() {
      while (flag.load(std::memory_order_acquire) < 2)
        ;
      assert(data[0] == 42);
    });

    t1.join();
    t2.join();
    t3.join();
  }
  return 0;
}