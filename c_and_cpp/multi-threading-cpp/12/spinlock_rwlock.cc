#include <atomic>

class RWSpinlock {
  // Encoding:
  //   0          = free
  //   -1         = writer holds lock
  //   positive N = N readers hold lock
  std::atomic<int> state_{0};

public:
  void lock_read() {
    for (;;) {
      // Spin while writer holds lock
      while (state_.load(std::memory_order_relaxed) < 0) {
        // Writer active, wait
      }

      // Try to increment reader count
      int expected = state_.load(std::memory_order_relaxed);
      if (expected >= 0) {
        if (state_.compare_exchange_weak(expected, expected + 1,
                                         std::memory_order_acquire,
                                         std::memory_order_relaxed)) {
          return;
        }
      }
      // Failed, retry
    }
  }

  void unlock_read() {
    // state_.fetch_sub(1, std::memory_order_release);
    // If readers ONLY read and never write protected data, relaxed is OK
    state_.fetch_sub(1, std::memory_order_relaxed);
  }

  void lock_write() {
    for (;;) {
      // Spin while anyone holds lock
      while (state_.load(std::memory_order_relaxed) != 0) {
        // Someone active, wait
      }

      // Try to acquire writer lock
      int expected = 0;
      if (state_.compare_exchange_weak(expected, -1, std::memory_order_acquire,
                                       std::memory_order_relaxed)) {
        return;
      }
      // Failed, retry
    }
  }

  void unlock_write() { state_.store(0, std::memory_order_release); }
};
