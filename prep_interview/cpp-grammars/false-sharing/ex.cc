#include <atomic>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <thread>
#include <vector>

struct Counter {
  alignas(64) std::atomic<uint64_t> a;
  alignas(64) std::atomic<uint64_t> b;
};

Counter counters;

void worker_a() {
  for (int i = 0; i < 100'000'000; ++i)
    counters.a.fetch_add(1, std::memory_order_relaxed);
}

void worker_b() {
  for (int i = 0; i < 100'000'000; ++i)
    counters.b.fetch_add(1, std::memory_order_relaxed);
}

int main() {
  std::thread t1(worker_a);
  std::thread t2(worker_b);
  t1.join();
  t2.join();

}
