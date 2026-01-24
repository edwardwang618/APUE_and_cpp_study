#ifndef MEMORY_POOL_HPP
#define MEMORY_POOL_HPP

#include "common.hpp"
#include <utility>

namespace memory {

// =============================================================================
// Pool (Fixed-Size Free List Allocator)
// =============================================================================
//
// How it works:
//   - Pre-allocate array of fixed-size slots
//   - Free slots are linked together in a list
//   - Alloc = pop from free list
//   - Free = push to free list
//
// Memory layout (initially):
//   +--------+--------+--------+--------+--------+
//   | slot 0 | slot 1 | slot 2 | slot 3 | slot 4 |
//   +--------+--------+--------+--------+--------+
//       |        ^  |     ^  |     ^  |     ^
//       +--------+  +-----+  +-----+  +-----+  -> nullptr
//
//   free_list_ points to slot 0
//   Each slot's "next" pointer points to next slot
//
// After alloc() returns slot 0:
//   +--------+--------+--------+--------+--------+
//   | USED   | slot 1 | slot 2 | slot 3 | slot 4 |
//   +--------+--------+--------+--------+--------+
//                 ^
//                 free_list_
//
// After free(slot 0):
//   +--------+--------+--------+--------+--------+
//   | slot 0 | slot 1 | slot 2 | slot 3 | slot 4 |
//   +--------+--------+--------+--------+--------+
//       ^         ^
//       |         |
//   free_list_    slot 0's next pointer
//
// Use cases:
//   - Order objects in trading systems
//   - Network connection objects
//   - Game entities
//   - Any fixed-size object with dynamic lifetime
//
// Pros:
//   - O(1) alloc and free
//   - No fragmentation (all slots same size)
//   - Good cache locality
//   - No external memory overhead (free list embedded in slots)
//
// Cons:
//   - Only works for single type/size
//   - Must know max count upfront
//
// =============================================================================

template <typename T> class Pool {
private:
  // -------------------------------------------------------------------------
  // The Slot union trick:
  //
  // When slot is FREE:  use "next" pointer for free list
  // When slot is USED:  use "object" storage for the actual T
  //
  // This means zero overhead - we don't need separate bookkeeping memory.
  // The free list is embedded in the unused slots themselves.
  // -------------------------------------------------------------------------
  union Slot {
    T object;
    Slot *next;

    // Must have trivial ctor/dtor since union contains non-trivial T
    Slot() {}  // don't initialize anything
    ~Slot() {} // don't destroy anything (we manage T's lifetime manually)
  };

  Slot *memory_;    // the array of slots
  Slot *free_list_; // head of free list
  size_t capacity_; // total number of slots

public:
  explicit Pool(size_t count) : capacity_(count) {
    memory_ = static_cast<Slot *>(mmap_alloc(sizeof(Slot) * count));

    // Build free list: each slot points to the next
    free_list_ = memory_;
    for (size_t i = 0; i < count - 1; i++) {
      memory_[i].next = &memory_[i + 1];
    }
    memory_[count - 1].next = nullptr;
  }

  ~Pool() {
    // Warning: doesn't call destructors on any live objects!
    // User is responsible for destroying all objects before Pool destructs.
    mmap_free(memory_, sizeof(Slot) * capacity_);
  }

  // Non-copyable, non-movable
  Pool(const Pool &) = delete;
  Pool &operator=(const Pool &) = delete;
  Pool(Pool &&) = delete;
  Pool &operator=(Pool &&) = delete;

  // -------------------------------------------------------------------------
  // Raw alloc - returns uninitialized memory
  // -------------------------------------------------------------------------
  T *alloc() {
    if (!free_list_)
      return nullptr;

    Slot *slot = free_list_;
    free_list_ = slot->next; // pop from list
    return reinterpret_cast<T *>(slot);
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
  // Destroy - destruct + return to pool
  // -------------------------------------------------------------------------
  void destroy(T *p) {
    if (!p)
      return;

    p->~T(); // call destructor

    Slot *slot = reinterpret_cast<Slot *>(p);
    slot->next = free_list_; // push to list
    free_list_ = slot;
  }

  // -------------------------------------------------------------------------
  // Free - return to pool WITHOUT calling destructor
  // Only use for trivially destructible types!
  // -------------------------------------------------------------------------
  void free(T *p) {
    if (!p)
      return;

    Slot *slot = reinterpret_cast<Slot *>(p);
    slot->next = free_list_;
    free_list_ = slot;
  }

  size_t capacity() const { return capacity_; }
};

} // namespace memory

#endif // MEMORY_POOL_HPP