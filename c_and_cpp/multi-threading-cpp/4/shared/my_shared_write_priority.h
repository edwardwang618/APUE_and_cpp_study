#include <condition_variable>
#include <mutex>

class SharedMutexWriterPriority {
private:
  std::mutex mtx_;
  std::condition_variable reader_cv_;
  std::condition_variable writer_cv_;
  int readers_ = 0;
  int waiting_writers_ = 0;
  bool writer_ = false;

public:
  void lock_shared() {
    std::unique_lock<std::mutex> lock(mtx_);
    // Wait if writer active OR writers waiting (priority to writers)
    reader_cv_.wait(lock,
                    [this]() { return !writer_ && waiting_writers_ == 0; });
    ++readers_;
  }

  void unlock_shared() {
    std::unique_lock<std::mutex> lock(mtx_);
    --readers_;
    if (readers_ == 0) {
      writer_cv_.notify_one(); // Wake one waiting writer
    }
  }

  void lock() {
    std::unique_lock<std::mutex> lock(mtx_);
    ++waiting_writers_;
    writer_cv_.wait(lock, [this]() { return !writer_ && readers_ == 0; });
    --waiting_writers_;
    writer_ = true;
  }

  void unlock() {
    std::unique_lock<std::mutex> lock(mtx_);
    writer_ = false;
    if (waiting_writers_ > 0) {
      writer_cv_.notify_one(); // Wake another writer first
    } else {
      reader_cv_.notify_all(); // Wake all readers
    }
  }
};