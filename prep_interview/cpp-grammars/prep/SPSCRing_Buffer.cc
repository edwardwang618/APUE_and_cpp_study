#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <iostream>

// Single Threaded
template <typename T, size_t Capacity> class RingBuffer {
private:
  static_assert((Capacity & (Capacity - 1)) == 0,
                "Capacity should be power of 2");
  static constexpr size_t MASK = Capacity - 1;

  alignas(64) std::atomic<std::size_t> head;
  alignas(64) std::atomic<std::size_t> tail;
  alignas(64) std::array<T, Capacity> buffer;

public:
  bool push(const T &item) {
    const size_t cur_tail = tail.load(std::memory_order_relaxed);
    const size_t next_tail = (cur_tail + 1) & MASK;
    if (next_tail == head.load(std::memory_order_acquire)) {
      return false;
    }

    buffer[cur_tail] = item;
    tail.store(next_tail, std::memory_order_release);
    return true;
  }

  bool pop(T &item) {
    const size_t cur_head = head.load(std::memory_order_relaxed);
    if (cur_head == tail.load(std::memory_order_acquire)) {
      return false;
    }

    item = std::move(buffer[cur_head]);
    head.store((cur_head + 1) & MASK, std::memory_order_release);
    return true;
  }
};

int main() {}