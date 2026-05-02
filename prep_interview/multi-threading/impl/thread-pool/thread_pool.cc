#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
public:
  // Spawn n worker threads
  explicit ThreadPool(size_t n) : stop_(false) {
    for (size_t i = 0; i < n; i++) {
      workers_.emplace_back([this] { worker(); });
    }
  }

  // Graceful shutdown - wait for all tasks to complete
  ~ThreadPool() {
    {
      std::lock_guard<std::mutex> lk(mutex_);
      stop_ = true;
    }
    cv_.notify_all();
    for (auto &t : workers_) {
      t.join();
    }
  }

  // Add a task to the queue
  void submit(std::function<void()> task) {
    std::lock_guard<std::mutex> lk(mutex_);
    if (stop_) {
      return;
    }
    tasks_.push(task);
    cv_.notify_one();
  }

private:
  // The function each worker thread runs
  void worker() {
    while (true) {
      std::function<void()> work;
      {
        std::unique_lock<std::mutex> lk(mutex_);
        cv_.wait(lk, [this] { return stop_ || !tasks_.empty(); });
        if (stop_ && tasks_.empty()) {
          return;
        }
        work = std::move(tasks_.front());
        tasks_.pop();
      }
      work();
    }
  }

  std::vector<std::thread> workers_;
  std::queue<std::function<void()>> tasks_;

  std::mutex mutex_;
  std::condition_variable cv_;

  bool stop_; // signal for shutdown
};