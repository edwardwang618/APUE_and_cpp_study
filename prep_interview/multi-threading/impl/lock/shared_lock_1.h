#include <condition_variable>
#include <mutex>

class SharedMutex {
private:
  std::mutex mtx_;             // Internal mutex to protect state
  std::condition_variable cv_; // For threads to wait and wake up
  int readers_ = 0;            // Count of active readers
  bool writer_ = false;        // Is there an active writer?

public:
  // ============================================
  // Acquire shared (read) lock
  // Multiple threads can hold this simultaneously
  // ============================================
  void lock_shared() {
    std::unique_lock<std::mutex> lock(mtx_);

    // Wait until no writer is active
    // Readers can coexist with other readers
    // But must wait if a writer is working
    cv_.wait(lock, [this]() { return !writer_; });

    // Now safe to read, increment reader count
    ++readers_;

    // Note: lock releases here, other readers can enter
  }

  // ============================================
  // Release shared (read) lock
  // ============================================
  void unlock_shared() {
    std::unique_lock<std::mutex> lock(mtx_);

    // One less reader
    --readers_;

    // If I'm the last reader, wake up waiting writers
    if (readers_ == 0) {
      cv_.notify_all();
    }
  }

  // ============================================
  // Acquire exclusive (write) lock
  // Only one thread can hold this
  // No readers allowed during write
  // ============================================
  void lock() {
    std::unique_lock<std::mutex> lock(mtx_);

    // Wait until:
    // 1. No other writer is active
    // 2. No readers are reading
    cv_.wait(lock, [this]() { return !writer_ && readers_ == 0; });

    // Now I'm the exclusive writer
    writer_ = true;
  }

  // ============================================
  // Release exclusive (write) lock
  // ============================================
  void unlock() {
    std::unique_lock<std::mutex> lock(mtx_);

    // Done writing
    writer_ = false;

    // Wake up everyone (readers and writers)
    // They will compete for the lock
    cv_.notify_all();
  }
};

// ============================================
// RAII wrapper for shared lock
// Automatically locks on construction
// Automatically unlocks on destruction
// ============================================
template <typename Mutex> class SharedLock {
private:
  Mutex &mtx_;

public:
  // Constructor: acquire shared lock
  explicit SharedLock(Mutex &mtx) : mtx_(mtx) { mtx_.lock_shared(); }

  // Destructor: release shared lock
  // Called automatically when object goes out of scope
  ~SharedLock() { mtx_.unlock_shared(); }

  // Non-copyable (prevent double unlock)
  SharedLock(const SharedLock &) = delete;
  SharedLock &operator=(const SharedLock &) = delete;
};

// ============================================
// RAII wrapper for exclusive lock
// ============================================
template <typename Mutex> class UniqueLock {
private:
  Mutex &mtx_;

public:
  // Constructor: acquire exclusive lock
  explicit UniqueLock(Mutex &mtx) : mtx_(mtx) { mtx_.lock(); }

  // Destructor: release exclusive lock
  ~UniqueLock() { mtx_.unlock(); }

  // Non-copyable
  UniqueLock(const UniqueLock &) = delete;
  UniqueLock &operator=(const UniqueLock &) = delete;
};