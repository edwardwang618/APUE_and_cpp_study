#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>

template <typename T, size_t Cap> class SPMCQueue {
  alignas(64) std::atomic<size_t> head;
  alignas(64) std::atomic<size_t> tail;
  alignas(64) std::array<T, Cap + 1> buffer;
  alignas(64) std::array<std::atomic<bool>, Cap + 1> ready;

public:
  SPMCQueue() {
    for (auto &r : ready)
      r.store(true, std::memory_order_relaxed);
  }

  bool push(const T &val) {
    size_t pos = tail.load(std::memory_order_relaxed);
    size_t next = (pos + 1) % (Cap + 1);
    if (next == head.load(std::memory_order_relaxed))
      return false;

    ready[pos].wait(false, std::memory_order_relaxed);
    buffer[pos] = val;
    ready[pos].store(false, std::memory_order_relaxed);
    tail.store(next, std::memory_order_release);
    return true;
  }

  bool pop(T &val) {
    size_t pos, next;
    do {
      pos = head.load(std::memory_order_relaxed);
      if (pos == tail.load(std::memory_order_acquire))
        return false;
      next = (pos + 1) % (Cap + 1);
    } while (!head.compare_exchange_weak(pos, next, std::memory_order_relaxed,
                                         std::memory_order_relaxed));
    val = std::move(buffer[pos]);
    ready[pos].store(true, std::memory_order_release);
    ready[pos].notify_one();
    return true;
  }
};
