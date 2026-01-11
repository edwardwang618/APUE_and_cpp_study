#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

class TTASSpinlock {
  std::atomic<bool> locked_{false};

public:
  void lock() {
    for (;;) {
      // First: just READ (no cache line bouncing while spinning)
      while (locked_.load(std::memory_order_relaxed)) {
        // Spin on local cache - no bus traffic
      }

      // Then: TRY to acquire with atomic exchange
      if (!locked_.exchange(true, std::memory_order_acquire)) {
        // Successfully changed false → true
        return;
      }
      // Someone else got it first, go back to spinning
    }
  }

  void unlock() { locked_.store(false, std::memory_order_release); }
};

class Counter {
  TTASSpinlock lock_;
  int value_ = 0;

public:
  void increment() {
    std::lock_guard<TTASSpinlock> guard(lock_);
    ++value_;
  }

  int get() const { return value_; }
};

int main() {
  Counter counter;
  constexpr int num_threads = 8;
  constexpr int increments_per_thread = 100000;

  std::vector<std::thread> threads;

  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([&counter]() {
      for (int j = 0; j < increments_per_thread; ++j) {
        counter.increment();
      }
    });
  }

  for (auto &t : threads) {
    t.join();
  }

  std::cout << "Final count: " << counter.get() << std::endl;
  std::cout << "Expected: " << num_threads * increments_per_thread << std::endl;

  return 0;
}
