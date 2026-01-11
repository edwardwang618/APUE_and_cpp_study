#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

class Spinlock {
private:
  std::atomic_flag flag_ = ATOMIC_FLAG_INIT; // Starts as false (unlocked)

public:
  void lock() {
    // Keep trying until we successfully acquire the lock
    while (flag_.test_and_set(std::memory_order_acquire))
      // test_and_set returned true, meaning it was already locked
      // Keep spinning...
      ;

    // test_and_set returned false, meaning it WAS unlocked
    // We changed it to locked, so we now own it
  }

  void unlock() { flag_.clear(std::memory_order_release); }
};

class Counter {
  Spinlock lock_;
  int value_ = 0;

public:
  void increment() {
    std::lock_guard<Spinlock> guard(lock_);
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
