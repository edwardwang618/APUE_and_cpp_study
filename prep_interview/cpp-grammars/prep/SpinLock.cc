#include <atomic>
#include <emmintrin.h>

class SpinLock {
private:
  // true: locked
  std::atomic<bool> flag{false};

public:
  SpinLock() {}
  SpinLock(const SpinLock &) = delete;
  SpinLock &operator=(const SpinLock &) = delete;
  SpinLock(SpinLock &&) = delete;
  SpinLock &operator=(SpinLock &&) = delete;

  void lock() {
    while (true) {
      while (flag.load(std::memory_order_relaxed)) {
        _mm_pause();
      }

      if (!flag.exchange(true, std::memory_order_acquire)) {
        return;
      }
    }
  }

  bool try_lock() {
    return !flag.load(std::memory_order_relaxed) &&
           !flag.exchange(true, std::memory_order_acquire);
  }

  void unlock() { flag.store(false, std::memory_order_release); }
};

class SpinLockGuard {
public:
  explicit SpinLockGuard(SpinLock &lk) : lock(lk) { lock.lock(); }
  SpinLockGuard(const SpinLockGuard &) = delete;
  SpinLockGuard &operator=(const SpinLockGuard &) = delete;
  ~SpinLockGuard() { lock.unlock(); }

private:
  SpinLock &lock;
};

int main() {}