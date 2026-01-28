#include <atomic>
#include <immintrin.h>

class Spinlock {
  std::atomic_flag flag = ATOMIC_FLAG_INIT;

public:
  void lock() {
    while (true) {
      while (flag.test(std::memory_order_relaxed))
        _mm_pause();

      if (!flag.test_and_set(std::memory_order_acquire))
        return;
    }
  }

  void unlock() { flag.clear(std::memory_order_release); }
};