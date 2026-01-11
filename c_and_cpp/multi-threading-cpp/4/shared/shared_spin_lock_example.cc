#include <atomic>
#include <chrono>
#include <immintrin.h>
#include <iostream>
#include <thread>
#include <vector>

class SpinSharedMutex {
private:
  std::atomic<uint32_t> state_{0};
  static constexpr uint32_t WRITER_BIT = 1U << 31;

public:
  void lock_shared() {
    while (true) {
      uint32_t expected = state_.load(std::memory_order_acquire);
      if (!(expected & WRITER_BIT)) {
        if (state_.compare_exchange_weak(expected, expected + 1,
                                         std::memory_order_acquire,
                                         std::memory_order_relaxed)) {
          return;
        }
      }
      _mm_pause();
    }
  }

  void unlock_shared() { state_.fetch_sub(1, std::memory_order_release); }

  void lock() {
    while (true) {
      uint32_t expected = 0;
      if (state_.compare_exchange_weak(expected, WRITER_BIT,
                                       std::memory_order_acquire,
                                       std::memory_order_relaxed)) {
        return;
      }
      _mm_pause();
    }
  }

  void unlock() { state_.store(0, std::memory_order_release); }
};

template <typename Mutex> class SharedLock {
  Mutex &mtx_;

public:
  explicit SharedLock(Mutex &mtx) : mtx_(mtx) { mtx_.lock_shared(); }
  ~SharedLock() { mtx_.unlock_shared(); }
};

template <typename Mutex> class UniqueLock {
  Mutex &mtx_;

public:
  explicit UniqueLock(Mutex &mtx) : mtx_(mtx) { mtx_.lock(); }
  ~UniqueLock() { mtx_.unlock(); }
};

// Test
int main() {
  SpinSharedMutex mtx;
  int shared_data = 0;
  std::atomic<int> read_count{0};
  std::atomic<int> write_count{0};

  auto start = std::chrono::high_resolution_clock::now();

  std::vector<std::thread> threads;

  // 4 readers
  for (int i = 0; i < 4; ++i) {
    threads.emplace_back([&]() {
      for (int j = 0; j < 100000; ++j) {
        SharedLock<SpinSharedMutex> lock(mtx);
        volatile int x = shared_data; // Read
        (void)x;
        ++read_count;
      }
    });
  }

  // 2 writers
  for (int i = 0; i < 2; ++i) {
    threads.emplace_back([&]() {
      for (int j = 0; j < 10000; ++j) {
        UniqueLock<SpinSharedMutex> lock(mtx);
        ++shared_data; // Write
        ++write_count;
      }
    });
  }

  for (auto &t : threads) {
    t.join();
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  std::cout << "Reads: " << read_count << "\n";
  std::cout << "Writes: " << write_count << "\n";
  std::cout << "Final value: " << shared_data << "\n";
  std::cout << "Time: " << duration.count() << "ms\n";

  return 0;
}