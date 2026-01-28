#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <immintrin.h>

template <typename T, size_t Cap> class MPSCQueue {
  alignas(64) std::atomic<size_t> head{0};
  alignas(64) std::atomic<size_t> tail{0};
  alignas(64) std::array<T, Cap + 1> buffer;
  alignas(64) std::array<std::atomic<bool>, Cap + 1> ready;

public:
  MPSCQueue() {
    for (auto &r : ready)
      r.store(false, std::memory_order_relaxed);
  }

  bool push(const T &val) {
    size_t pos, next;
    do {
      pos = tail.load(std::memory_order_relaxed);
      next = (pos + 1) % (Cap + 1);
      if (next == head.load(std::memory_order_acquire))
        return false;
    } while (!tail.compare_exchange_weak(pos, next, std::memory_order_relaxed,
                                         std::memory_order_relaxed));
    buffer[pos] = val;
    ready[pos].store(true, std::memory_order_release);
    return true;
  }

  bool pop(T &val) {
    size_t pos = head.load(std::memory_order_relaxed);
    size_t next = (pos + 1) % (Cap + 1);
    if (pos == tail.load(std::memory_order_acquire))
      return false;

    while (!ready[pos].load(std::memory_order_acquire))
      _mm_pause();
    val = std::move(buffer[pos]);
    ready[pos].store(false, std::memory_order_relaxed);
    head.store(next, std::memory_order_release);
    return true;
  }
};

int main() {}