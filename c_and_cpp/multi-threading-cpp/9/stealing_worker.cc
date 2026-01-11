#include <deque>
#include <functional>
#include <random>
#include <thread>

class StealingWorker {
public:
  StealingWorker(std::string name, std::vector<StealingWorker *> *all_workers)
      : name_(std::move(name)), all_workers_(all_workers), stop_(false),
        rng_(std::random_device{}()) {
    worker_ = std::thread([this]() { run(); });
  }

  ~StealingWorker() {
    {
      std::lock_guard<std::mutex> lk(mu_);
      stop_ = true;
    }
    cv_.notify_one();
    worker_.join();
  }

  void add_task(std::function<void()> task) {
    {
      std::lock_guard<std::mutex> lk(mu_);
      local_queue_.push_back(std::move(task));
    }
    cv_.notify_one();
  }

  // Called by other workers to steal
  bool try_give_task(std::function<void()> &task) {
    std::lock_guard<std::mutex> lk(mu_);
    if (local_queue_.empty()) {
      return false;
    }
    // Steal from the BACK (opposite end from owner)
    task = std::move(local_queue_.back());
    local_queue_.pop_back();
    return true;
  }

private:
  void run() {
    while (true) {
      std::function<void()> task;

      // First, try to get from local queue
      {
        std::unique_lock<std::mutex> lk(mu_);

        if (local_queue_.empty() && !stop_) {
          // Try stealing before blocking
          lk.unlock();
          if (try_steal(task)) {
            task();
            continue;
          }
          lk.lock();

          // Wait with timeout to periodically retry stealing
          cv_.wait_for(lk, std::chrono::milliseconds(1),
                       [this]() { return !local_queue_.empty() || stop_; });
        }

        if (stop_ && local_queue_.empty()) {
          break;
        }

        if (!local_queue_.empty()) {
          // Take from the FRONT (owner takes from front)
          task = std::move(local_queue_.front());
          local_queue_.pop_front();
        }
      }

      if (task) {
        task();
      }
    }
  }

  bool try_steal(std::function<void()> &task) {
    if (all_workers_->size() <= 1) {
      return false;
    }

    // Random victim selection
    std::uniform_int_distribution<size_t> dist(0, all_workers_->size() - 1);

    // Try a few random victims
    for (int attempts = 0; attempts < 3; ++attempts) {
      size_t victim_idx = dist(rng_);
      StealingWorker *victim = (*all_workers_)[victim_idx];

      if (victim == this) {
        continue; // Don't steal from self
      }

      if (victim->try_give_task(task)) {
        return true;
      }
    }

    return false;
  }

  std::string name_;
  std::vector<StealingWorker *> *all_workers_;
  bool stop_;
  std::thread worker_;
  std::deque<std::function<void()>>
      local_queue_; // Deque for double-ended access
  std::mutex mu_;
  std::condition_variable cv_;
  std::mt19937 rng_;
};