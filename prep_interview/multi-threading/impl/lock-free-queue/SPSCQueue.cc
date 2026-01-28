#include <array>
#include <atomic>
#include <cstddef>
#include <iostream>
#include <thread>

template <typename T, size_t Cap> class SPSCQueue {
  alignas(64) std::atomic<size_t> head{0};
  alignas(64) std::atomic<size_t> tail{0};
  alignas(64) size_t cached_head;
  alignas(64) size_t cached_tail;
  alignas(64) std::array<T, Cap + 1> buffer;

public:
  bool push(const T &val) {
    size_t pos = tail.load(std::memory_order_relaxed);
    size_t next = (pos + 1) % (Cap + 1);
    if (next == head.load(std::memory_order_acquire))
      return false;

    buffer[pos] = val;
    tail.store(next, std::memory_order_release);
    return true;
  }

  bool pop(T &val) {
    size_t pos = head.load(std::memory_order_relaxed);
    if (pos == tail.load(std::memory_order_acquire))
      return false;

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

int main() {}