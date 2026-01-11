#include <atomic>
#include <iostream>

int main() {
  // Check at compile time
  static_assert(std::atomic<int>::is_always_lock_free,
                "int atomics must be lock-free for this code");

  // Typical results on x86-64:
  std::cout << std::atomic<int>::is_always_lock_free
            << std::endl; // true (4 bytes)
  std::cout << std::atomic<long>::is_always_lock_free
            << std::endl; // true (8 bytes)
  std::cout << std::atomic<__int128>::is_always_lock_free
            << std::endl; // false (too big)

  struct Big {
    char data[100];
  };
  std::cout << std::atomic<Big>::is_always_lock_free
            << std::endl; // false (too big)
}