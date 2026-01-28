#include <array>
#include <atomic>
#include <cstddef>

template <typename T, size_t Cap> class SPSCQueue {
  alignas(64) std::atomic<size_t> head{0};
  alignas(64) std::atomic<size_t> tail{0};
  alignas(64) size_t cached_head{0};
  alignas(64) size_t cached_tail{0};
  alignas(64) std::array<T, Cap + 1> buffer;

public:
  bool push(const T &val) {
    size_t pos = tail.load(std::memory_order_relaxed);
    size_t next = (pos + 1) % (Cap + 1);
    if (next == cached_head) {
      cached_head = head.load(std::memory_order_acquire);
      if (next == cached_head)
        return false;
    }

    buffer[pos] = val;
    tail.store(next, std::memory_order_release);
    return true;
  }

  bool pop(T &val) {
    size_t pos = head.load(std::memory_order_relaxed);
    if (pos == cached_tail) {
      cached_tail = tail.load(std::memory_order_acquire);
      if (pos == cached_tail)
        return false;
    }

    val = std::move(buffer[pos]);
    size_t next = (pos + 1) % (Cap + 1);
    head.store(next, std::memory_order_release);
    return true;
  }

  size_t size() const {
    return (tail.load(std::memory_order_acquire) -
            head.load(std::memory_order_acquire) + Cap + 1) %
           (Cap + 1);
  }
};
