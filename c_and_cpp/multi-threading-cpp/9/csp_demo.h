#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <vector>

// ============================================
// Channel: The core of CSP model
// A typed, thread-safe, bounded channel
// Similar to Go's buffered channel
// ============================================
template <typename T> class Channel {
public:
  // ============================================
  // Constructor
  // capacity = 0 means unbuffered (synchronous)
  // capacity > 0 means buffered (async up to capacity)
  // ============================================
  explicit Channel(size_t capacity = 0) : capacity_(capacity), closed_(false) {}

  // ============================================
  // Send a value into the channel
  // Blocks if buffer is full
  // Returns false if channel is closed
  // ============================================
  bool send(T value) {
    std::unique_lock<std::mutex> lock(mu_);

    // Wait until:
    // 1. There's room in buffer, OR
    // 2. Channel is closed
    // For unbuffered (capacity_=0): wait until someone is receiving
    cv_send_.wait(lock, [this]() {
      return closed_ || queue_.size() < capacity_ ||
             (capacity_ == 0 && receivers_waiting_ > 0);
    });

    // Can't send to closed channel
    if (closed_) {
      return false;
    }

    // Put value in queue
    queue_.push(std::move(value));

    // Wake up one waiting receiver
    cv_recv_.notify_one();

    // For unbuffered channel: wait until value is taken
    if (capacity_ == 0) {
      cv_send_.wait(lock, [this]() { return queue_.empty() || closed_; });
    }

    return true;
  }

  // ============================================
  // Receive a value from the channel
  // Blocks if buffer is empty
  // Returns std::nullopt if channel is closed and empty
  // ============================================
  std::optional<T> recv() {
    std::unique_lock<std::mutex> lock(mu_);

    // Increment waiting receivers (for unbuffered channel)
    ++receivers_waiting_;
    cv_send_.notify_one(); // Wake sender for unbuffered case

    // Wait until:
    // 1. There's data in buffer, OR
    // 2. Channel is closed
    cv_recv_.wait(lock, [this]() { return !queue_.empty() || closed_; });

    --receivers_waiting_;

    // Channel closed and empty
    if (queue_.empty()) {
      return std::nullopt;
    }

    // Get value from queue
    T value = std::move(queue_.front());
    queue_.pop();

    // Wake up one waiting sender
    cv_send_.notify_one();

    return value;
  }

  // ============================================
  // Close the channel
  // No more sends allowed, but can still receive remaining data
  // ============================================
  void close() {
    std::lock_guard<std::mutex> lock(mu_);
    closed_ = true;
    cv_send_.notify_all(); // Wake all senders
    cv_recv_.notify_all(); // Wake all receivers
  }

  // ============================================
  // Check if channel is closed
  // ============================================
  bool is_closed() const {
    std::lock_guard<std::mutex> lock(mu_);
    return closed_;
  }

private:
  mutable std::mutex mu_;
  std::condition_variable cv_send_; // Senders wait on this
  std::condition_variable cv_recv_; // Receivers wait on this
  std::queue<T> queue_;             // The buffer
  size_t capacity_;                 // Max buffer size (0 = unbuffered)
  bool closed_;                     // Is channel closed?
  size_t receivers_waiting_ = 0;    // Count of waiting receivers
};

// ============================================
// Convenience operator for sending (like Go's ch <- value)
// ============================================
template <typename T> Channel<T> &operator<<(Channel<T> &ch, T value) {
  ch.send(std::move(value));
  return ch;
}

// ============================================
// Convenience operator for receiving (like Go's value = <-ch)
// ============================================
template <typename T> Channel<T> &operator>>(Channel<T> &ch, T &value) {
  auto result = ch.recv();
  if (result) {
    value = std::move(*result);
  }
  return ch;
}
