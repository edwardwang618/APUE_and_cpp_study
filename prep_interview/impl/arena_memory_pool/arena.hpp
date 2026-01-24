#ifndef MEMORY_ARENA_HPP
#define MEMORY_ARENA_HPP

#include "common.hpp"
#include <utility>

namespace memory {

// =============================================================================
// Arena (Bump Allocator)
// =============================================================================
//
// How it works:
//   - Pre-allocate a large contiguous block of memory
//   - Allocation = just bump a pointer forward
//   - No individual free - reset everything at once
//
// Memory layout:
//   +------------------------------------------------------------------+
//   | allocated | allocated | allocated |        free space           |
//   +------------------------------------------------------------------+
//   ^                                   ^                              ^
//   base_                               current_                       end_
//
// Use cases:
//   - Per-frame game allocations (reset each frame)
//   - Request processing (reset after each request)
//   - Parsing (allocate AST nodes, reset after done)
//   - Batch processing market data
//
// Pros:
//   - O(1) allocation (just pointer bump)
//   - Zero fragmentation
//   - Cache-friendly (sequential memory)
//   - No per-object overhead
//
// Cons:
//   - Can't free individual objects
//   - Must reset all at once
//   - Need to estimate max size upfront
//
// =============================================================================

class Arena {
private:
  char *base_;    // start of memory block
  char *current_; // next allocation starts here
  char *end_;     // end of memory block

public:
  explicit Arena(size_t size) {
    base_ = static_cast<char *>(mmap_alloc(size));
    current_ = base_;
    end_ = base_ + size;
  }

  ~Arena() { mmap_free(base_, end_ - base_); }

  // Non-copyable, non-movable
  Arena(const Arena &) = delete;
  Arena &operator=(const Arena &) = delete;
  Arena(Arena &&) = delete;
  Arena &operator=(Arena &&) = delete;

  // -------------------------------------------------------------------------
  // Raw allocation with alignment
  // -------------------------------------------------------------------------
  //
  // Example with alignment = 8:
  //   current_ = 0x1003
  //   aligned  = (0x1003 + 7) & ~7 = 0x1008
  //   return 0x1008, advance current_ to 0x1008 + size
  //
  void *alloc(size_t size, size_t alignment = 8) {
    uintptr_t current = reinterpret_cast<uintptr_t>(current_);
    uintptr_t aligned = align_up(current, alignment);
    char *result = reinterpret_cast<char *>(aligned);

    if (result + size > end_) {
      return nullptr; // out of memory
    }

    current_ = result + size;
    return result;
  }

  // -------------------------------------------------------------------------
  // Typed allocation - allocate and construct
  // -------------------------------------------------------------------------
  template <typename T, typename... Args> T *create(Args &&...args) {
    void *mem = alloc(sizeof(T), alignof(T));
    if (!mem)
      return nullptr;

    // Placement new: construct T in pre-allocated memory
    return new (mem) T(std::forward<Args>(args)...);
  }

  // -------------------------------------------------------------------------
  // Array allocation - allocate and default-construct each element
  // -------------------------------------------------------------------------
  template <typename T> T *createArray(size_t count) {
    void *mem = alloc(sizeof(T) * count, alignof(T));
    if (!mem)
      return nullptr;

    T *arr = static_cast<T *>(mem);
    for (size_t i = 0; i < count; i++) {
      new (&arr[i]) T();
    }
    return arr;
  }

  // -------------------------------------------------------------------------
  // Reset - "free" all allocations at once
  // -------------------------------------------------------------------------
  // Note: Does NOT call destructors!
  // Only use with trivially destructible types, or call destructors manually.
  //
  void reset() { current_ = base_; }

  // Stats
  size_t used() const { return current_ - base_; }
  size_t remaining() const { return end_ - current_; }
  size_t capacity() const { return end_ - base_; }
};

} // namespace memory

#endif // MEMORY_ARENA_HPP