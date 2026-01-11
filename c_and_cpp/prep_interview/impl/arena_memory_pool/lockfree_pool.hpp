#ifndef MEMORY_LOCKFREE_POOL_HPP
#define MEMORY_LOCKFREE_POOL_HPP

#include <atomic>
#include <cstddef>
#include <new>
#include <utility>

namespace memory {

// =============================================================================
// Lock-Free Pool
// =============================================================================
//
// How it works:
//   - Fixed array of slots, each with an "occupied" atomic flag
//   - Alloc: scan for unoccupied slot, CAS to claim it
//   - Free: just clear the flag
//
// Why not use a lock-free free list?
//   The ABA problem makes lock-free linked lists complex.
//   This bitmap-style approach is simpler and often faster for small pools.
//
// Memory layout:
//   +-------------------+-------------------+-------------------+
//   | occupied: false   | occupied: true    | occupied: false   |
//   | storage: [    ]   | storage: [Order]  | storage: [    ]   |
//   +-------------------+-------------------+-------------------+
//                            ^
//                         in use
//
// Cache line alignment (alignas(64)):
//   - Prevents false sharing between alloc_idx_ and free_idx_
//   - Each atomic on its own cache line
//
// Use cases:
//   - Multi-threaded order pools
//   - Lock-free message queues
//   - High-frequency trading where mutex latency is unacceptable
//
// Limitations:
//   - Capacity must be power of 2 (for fast modulo via bitwise AND)
//   - Linear scan when pool is nearly full
//   - Not optimal for very large pools
//
// =============================================================================

template <typename T, size_t Capacity> class LockFreePool {
  static_assert((Capacity & (Capacity - 1)) == 0,
                "Capacity must be power of 2 for fast indexing");

private:
  struct Slot {
    std::atomic<bool> occupied{false};
    alignas(T) char storage[sizeof(T)]; // raw storage for T
  };

  // Each on separate cache line to avoid false sharing
  alignas(64) Slot slots_[Capacity];
  alignas(64) std::atomic<size_t> alloc_hint_{
      0}; // hint for where to start scanning

public:
  LockFreePool() = default;

  ~LockFreePool() {
    // Warning: doesn't destroy live objects
    // User must destroy all objects before pool destructs
  }

  // Non-copyable, non-movable
  LockFreePool(const LockFreePool &) = delete;
  LockFreePool &operator=(const LockFreePool &) = delete;
  LockFreePool(LockFreePool &&) = delete;
  LockFreePool &operator=(LockFreePool &&) = delete;

  // -------------------------------------------------------------------------
  // Alloc - find and claim an unoccupied slot
  // -------------------------------------------------------------------------
  T *alloc() {
    size_t start = alloc_hint_.load(std::memory_order_relaxed);

    // Scan all slots looking for a free one
    for (size_t i = 0; i < Capacity; i++) {
      size_t idx = (start + i) & (Capacity - 1); // fast modulo

      bool expected = false;
      // Try to atomically set occupied from false to true
      if (slots_[idx].occupied.compare_exchange_strong(
              expected, true,
              std::memory_order_acquire,  // success: acquire
              std::memory_order_relaxed)) // failure: relaxed
      {
        // Update hint for next allocation
        alloc_hint_.store(idx + 1, std::memory_order_relaxed);
        return reinterpret_cast<T *>(slots_[idx].storage);
      }
    }
    return nullptr; // pool exhausted
  }

  // -------------------------------------------------------------------------
  // Create - alloc + construct
  // -------------------------------------------------------------------------
  template <typename... Args> T *create(Args &&...args) {
    T *mem = alloc();
    if (!mem)
      return nullptr;
    return new (mem) T(std::forward<Args>(args)...);
  }

  // -------------------------------------------------------------------------
  // Destroy - destruct + release slot
  // -------------------------------------------------------------------------
  void destroy(T *p) {
    if (!p)
      return;

    p->~T();

    // Find which slot this pointer belongs to
    // pointer arithmetic: (p - base) gives slot index
    char *raw = reinterpret_cast<char *>(p);
    size_t idx =
        (reinterpret_cast<Slot *>(raw - offsetof(Slot, storage)) - slots_);

    // Release the slot
    slots_[idx].occupied.store(false, std::memory_order_release);
  }

  size_t capacity() const { return Capacity; }
};

} // namespace memory

#endif // MEMORY_LOCKFREE_POOL_HPP