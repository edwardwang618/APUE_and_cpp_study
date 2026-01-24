#include <array>
#include <atomic>
#include <iostream>
#include <thread>

template <typename T, size_t Cap> class SPSCQueue {
  std::array<T, Cap + 1> buffer; // Why +1?
  std::atomic<size_t> head{0};   // read position (consumer)
  std::atomic<size_t> tail{0};   // write position (producer)

public:
  bool push(const T &val) {
    // TODO
    if (head.load() == tail.load()) return false;
    int next = tail.load(std::memory_order_acquire) + 1;
    next = (next + 1) % Cap;
    buffer[next] = val;
    return true;
  }

  bool pop(T &val) {
    // TODO
    val = buffer[head.load()];
    head.fetch_add(1, std::memory_order_release);
    return true;
  }
};

int main() {}