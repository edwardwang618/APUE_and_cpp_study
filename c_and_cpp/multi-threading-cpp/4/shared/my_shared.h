#include <condition_variable>
#include <mutex>

class SharedMutex {
private:
  std::mutex mtx_;
  std::condition_variable cv_;
  int readers_ = 0;     // Count of active readers
  bool writer_ = false; // Is there an active writer?

public:
  // Acquire shared (read) lock
  void lock_shared() {
    std::unique_lock<std::mutex> lock(mtx_);
    // Wait until no writer is active
    cv_.wait(lock, [this]() { return !writer_; });
    ++readers_;
  }

  // Release shared (read) lock
  void unlock_shared() {
    std::unique_lock<std::mutex> lock(mtx_);
    --readers_;
    if (readers_ == 0) {
      cv_.notify_all(); // Wake up waiting writers
    }
  }

  // Acquire exclusive (write) lock
  void lock() {
    std::unique_lock<std::mutex> lock(mtx_);
    // Wait until no writer AND no readers
    cv_.wait(lock, [this]() { return !writer_ && readers_ == 0; });
    writer_ = true;
  }

  // Release exclusive (write) lock
  void unlock() {
    std::unique_lock<std::mutex> lock(mtx_);
    writer_ = false;
    cv_.notify_all(); // Wake up waiting readers and writers
  }
};

// RAII wrapper for shared lock
template <typename Mutex> class SharedLock {
private:
  Mutex &mtx_;

public:
  explicit SharedLock(Mutex &mtx) : mtx_(mtx) { mtx_.lock_shared(); }

  ~SharedLock() { mtx_.unlock_shared(); }

  // Non-copyable
  SharedLock(const SharedLock &) = delete;
  SharedLock &operator=(const SharedLock &) = delete;
};

// RAII wrapper for exclusive lock
template <typename Mutex> class UniqueLock {
private:
  Mutex &mtx_;

public:
  explicit UniqueLock(Mutex &mtx) : mtx_(mtx) { mtx_.lock(); }

  ~UniqueLock() { mtx_.unlock(); }

  // Non-copyable
  UniqueLock(const UniqueLock &) = delete;
  UniqueLock &operator=(const UniqueLock &) = delete;
};
