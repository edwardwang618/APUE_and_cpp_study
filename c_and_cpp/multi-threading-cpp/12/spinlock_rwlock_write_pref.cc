#include <atomic>
#include <cstdint>

class RWSpinlockWriterPrefer_OneAtomic {
  static constexpr uint32_t WRITER_HELD    = 1u << 31;
  static constexpr uint32_t WRITER_WAITING = 1u << 30;
  static constexpr uint32_t READER_MASK    = WRITER_WAITING - 1; // low 30 bits

  std::atomic<uint32_t> s_{0};

public:
  void lock_read() {
    uint32_t cur = s_.load(std::memory_order_relaxed);
    for (;;) {
      // Block if writer is waiting or holding
      if (cur & (WRITER_WAITING | WRITER_HELD)) {
        cur = s_.load(std::memory_order_relaxed);
        continue;
      }
      // Try to increment reader count
      if ((cur & READER_MASK) == READER_MASK) [[unlikely]] {
        // reader overflow (practically impossible)
        cur = s_.load(std::memory_order_relaxed);
        continue;
      }

      uint32_t desired = cur + 1;
      if (s_.compare_exchange_weak(cur, desired,
                                   std::memory_order_acquire,
                                   std::memory_order_relaxed)) {
        return;
      }
      // on failure, cur updated
    }
  }

  void unlock_read() {
    // release: reader's critical section completes-before writer acquiring
    s_.fetch_sub(1, std::memory_order_release);
  }

  void lock_write() {
    // Announce writer waiting. RMW on same atomic readers must CAS against.
    s_.fetch_or(WRITER_WAITING, std::memory_order_relaxed);

    for (;;) {
      // We can acquire only when: waiting set, no readers, not held.
      uint32_t expected = WRITER_WAITING;
      if (s_.compare_exchange_weak(expected, WRITER_HELD,
                                   std::memory_order_acquire,
                                   std::memory_order_relaxed)) {
        return;
      }
      // Otherwise spin; expected updated with current value.
    }
  }

  void unlock_write() {
    // release: publish writer's critical section to future acquirers
    s_.store(0, std::memory_order_release);
  }
};