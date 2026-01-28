#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>

template <typename T, size_t Capacity> class BoundedQueue {
  mutable std::mutex mu_;
  std::queue<T> q_;
  std::condition_variable cv_not_full_;
  std::condition_variable cv_not_empty_;
  bool shutdown_ = false;

public:
  BoundedQueue() = default;

  // Non-copyable when using shutdown (simpler)
  BoundedQueue(const BoundedQueue &) = delete;
  BoundedQueue &operator=(const BoundedQueue &) = delete;

  void shutdown() {
    std::lock_guard<std::mutex> lk(mu_);
    shutdown_ = true;
    cv_not_empty_.notify_all();
    cv_not_full_.notify_all();
  }

  bool is_shutdown() const {
    std::lock_guard<std::mutex> lk(mu_);
    return shutdown_;
  }

  // Blocking push - returns false if shutdown
  bool push(const T &v) {
    std::unique_lock<std::mutex> lk(mu_);
    cv_not_full_.wait(lk, [this] { return q_.size() < Capacity || shutdown_; });

    if (shutdown_)
      return false;

    q_.push(v);
    cv_not_empty_.notify_one();
    return true;
  }

  // Non-blocking push
  bool try_push(const T &v) {
    std::lock_guard<std::mutex> lk(mu_);
    if (shutdown_ || q_.size() >= Capacity)
      return false;

    q_.push(v);
    cv_not_empty_.notify_one();
    return true;
  }

  // Blocking pop - returns false if shutdown AND empty
  bool pop(T &v) {
    std::unique_lock<std::mutex> lk(mu_);
    cv_not_empty_.wait(lk, [this] { return !q_.empty() || shutdown_; });

    if (q_.empty())
      return false; // Shutdown and empty

    v = std::move(q_.front());
    q_.pop();
    cv_not_full_.notify_one();
    return true;
  }

  // Non-blocking pop
  bool try_pop(T &v) {
    std::lock_guard<std::mutex> lk(mu_);
    if (q_.empty())
      return false;

    v = std::move(q_.front());
    q_.pop();
    cv_not_full_.notify_one();
    return true;
  }

  // Timed pop
  template <typename Rep, typename Period>
  bool pop_for(T &v, std::chrono::duration<Rep, Period> timeout) {
    std::unique_lock<std::mutex> lk(mu_);

    if (!cv_not_empty_.wait_for(lk, timeout,
                                [this] { return !q_.empty() || shutdown_; })) {
      return false; // Timed out
    }

    if (q_.empty())
      return false; // Shutdown and empty

    v = std::move(q_.front());
    q_.pop();
    cv_not_full_.notify_one();
    return true;
  }

  // Optional-based pop (modern C++ style)
  std::optional<T> pop() {
    std::unique_lock<std::mutex> lk(mu_);
    cv_not_empty_.wait(lk, [this] { return !q_.empty() || shutdown_; });

    if (q_.empty())
      return std::nullopt;

    T v = std::move(q_.front());
    q_.pop();
    cv_not_full_.notify_one();
    return v;
  }
};

int main() {
  BoundedQueue<int, 5> bq;

  std::thread producer([&] {
    for (int i = 0; i < 10; i++) {
      bq.push(i);
      std::cout << "pushed " << i << "\n";
    }
    std::cout << "Producer done, shutting down\n";
    bq.shutdown();
  });

  std::thread consumer([&] {
    while (auto v = bq.pop()) {
      std::cout << "popped " << *v << "\n";
    }
    std::cout << "Consumer done\n";
  });

  producer.join();
  consumer.join();
}