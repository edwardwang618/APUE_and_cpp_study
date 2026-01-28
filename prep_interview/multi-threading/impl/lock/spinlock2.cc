#include <atomic>

class Spinlock {
  std::atomic<bool> flag{false};

public:
  void lock() {
    while (true) {
      flag.wait(true, std::memory_order_relaxed);

      if (!flag.exchange(true, std::memory_order_acquire))
        return;
    }
  }

  void unlock() {
    flag.store(false, std::memory_order_release);
    flag.notify_one();
  }
};

int main() {}