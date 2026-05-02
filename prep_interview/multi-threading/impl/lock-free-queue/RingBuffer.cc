#include <algorithm>
#include <array>
#include <atomic>
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

template <typename T, std::size_t Capacity, bool PlacementNew = true>
class RingBuffer {
public:
  static_assert(Capacity > 0 && (Capacity & (Capacity - 1)) == 0,
                "Capactiy must be power of 2");
  static_assert(
      PlacementNew || std::is_default_constructible_v<T>,
      "std::array (PlacementNew=false) needs T to be default constructible");
  RingBuffer() : head_(0), tail_(0) {}
  ~RingBuffer() {
    if constexpr (PlacementNew) {
      size_t head = head_.load(std::memory_order_relaxed);
      size_t tail = tail_.load(std::memory_order_relaxed);
      while (head != tail) {
        at(head)->~T();
        head = (head + 1) & (Capacity - 1);
      }
    }
  }

  RingBuffer(const RingBuffer &) = delete;
  RingBuffer(RingBuffer &&) = delete;
  RingBuffer &operator=(const RingBuffer &) = delete;
  RingBuffer &operator=(RingBuffer &&) = delete;

  template <typename U>
    requires std::constructible_from<T, U &&>
  bool Push(U &&value) {
    size_t tail = tail_.load(std::memory_order_relaxed);
    size_t next_tail = (tail + 1) & (Capacity - 1);
    if (next_tail == head_.load(std::memory_order_acquire)) {
      return false;
    }

    if constexpr (PlacementNew) {
      new (at(tail)) T(std::forward<U>(value));
    } else {
      *at(tail) = std::forward<U>(value);
    }

    tail_.store(next_tail, std::memory_order_release);
    return true;
  }

  template <typename U>
  bool Pop(T &value) {
    size_t head = head_.load(std::memory_order_relaxed);
    size_t next_head = (head + 1) & (Capacity - 1);
    if (head == tail_.load(std::memory_order_acquire)) {
      return false;
    }

    if constexpr (PlacementNew) {
      T *ptr = at(head);
      value = std::move(*ptr);
      ptr->~T();
    } else {
      value = std::move(*at(head));
    }
    head_.store(next_head, std::memory_order_release);
    return true;
  }

private:
  struct ByteBuffer {
    alignas(alignof(T)) std::byte data[sizeof(T) * Capacity];
  };

  T *at(size_t index) {
    if constexpr (PlacementNew) {
      return reinterpret_cast<T *>(storage_.data) + index;
    } else {
      return &storage_[index];
    }
  }

  using Storage =
      std::conditional_t<PlacementNew, ByteBuffer, std::array<T, Capacity>>;
  alignas(64) std::atomic<std::size_t> head_;
  alignas(64) std::atomic<std::size_t> tail_;
  alignas(64) Storage storage_;
};