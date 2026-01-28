#include <condition_variable>
#include <mutex>

class SharedMutexWriterPriority {
private:
  std::mutex mtx_;
  std::condition_variable reader_cv_; // Readers wait on this
  std::condition_variable writer_cv_; // Writers wait on this
  int readers_ = 0;                   // Count of active readers
  int waiting_writers_ = 0;           // Count of writers waiting in queue
  bool writer_ = false;               // Is there an active writer?

public:
  // ============================================
  // Acquire shared (read) lock
  // Writers have priority over new readers!
  // ============================================
  void lock_shared() {
    std::unique_lock<std::mutex> lock(mtx_);

    // Wait if:
    // 1. A writer is currently writing, OR
    // 2. Writers are waiting (give them priority!)
    //
    // This is the KEY difference from basic SharedMutex
    // New readers must wait for all waiting writers
    reader_cv_.wait(lock,
                    [this]() { return !writer_ && waiting_writers_ == 0; });

    // Safe to read now
    ++readers_;
  }

  // ============================================
  // Release shared (read) lock
  // ============================================
  void unlock_shared() {
    std::unique_lock<std::mutex> lock(mtx_);

    --readers_;

    // If I'm the last reader, wake up ONE waiting writer
    // (not all, just one - more efficient)
    if (readers_ == 0) {
      writer_cv_.notify_one();
    }
  }

  // ============================================
  // Acquire exclusive (write) lock
  // ============================================
  void lock() {
    std::unique_lock<std::mutex> lock(mtx_);

    // Register myself as waiting writer
    // This blocks new readers from entering!
    ++waiting_writers_;

    // Wait until no writer AND no readers
    writer_cv_.wait(lock, [this]() { return !writer_ && readers_ == 0; });

    // Got the lock, no longer waiting
    --waiting_writers_;
    writer_ = true;
  }

  // ============================================
  // Release exclusive (write) lock
  // ============================================
  void unlock() {
    std::unique_lock<std::mutex> lock(mtx_);

    writer_ = false;

    // Priority: wake writers first
    if (waiting_writers_ > 0) {
      // More writers waiting, let them go first
      writer_cv_.notify_one();
    } else {
      // No writers waiting, let all readers go
      reader_cv_.notify_all();
    }
  }
};
