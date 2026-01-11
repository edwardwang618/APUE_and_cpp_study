#ifndef MEMORY_COMMON_HPP
#define MEMORY_COMMON_HPP

#include <cstddef>
#include <cstdint>
#include <new>
#include <sys/mman.h>

namespace memory {

// Helper to allocate via mmap (avoids malloc, gets pages directly from OS)
inline void *mmap_alloc(size_t size) {
  void *ptr = mmap(nullptr,                     // let OS choose address
                   size,                        // size in bytes
                   PROT_READ | PROT_WRITE,      // readable and writable
                   MAP_PRIVATE | MAP_ANONYMOUS, // private, not backed by file
                   -1,                          // no file descriptor
                   0                            // no offset
  );

  if (ptr == MAP_FAILED) {
    throw std::bad_alloc();
  }
  return ptr;
}

inline void mmap_free(void *ptr, size_t size) { munmap(ptr, size); }

// Align pointer up to alignment boundary
inline uintptr_t align_up(uintptr_t ptr, size_t alignment) {
  return (ptr + alignment - 1) & ~(alignment - 1);
}

} // namespace memory

#endif // MEMORY_COMMON_HPP