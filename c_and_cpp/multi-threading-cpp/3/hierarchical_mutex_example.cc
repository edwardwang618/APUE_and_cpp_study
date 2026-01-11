#include <climits>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>

class hierarchical_mutex {
public:
  explicit hierarchical_mutex(unsigned long level)
      : hierarchy_level_(level), previous_hierarchy_level_(0) {}

  hierarchical_mutex(const hierarchical_mutex&) = delete;
  hierarchical_mutex& operator=(const hierarchical_mutex&) = delete;

  void lock() {
    check_for_hierarchy_violation();
    internal_mutex_.lock();
    update_hierarchy_value();
  }

  void unlock() {
    if (this_thread_hierarchy_value_ != hierarchy_level_) {
      throw std::logic_error("Mutex hierarchy violated on unlock");
    }
    this_thread_hierarchy_value_ = previous_hierarchy_level_;
    internal_mutex_.unlock();
  }

  bool try_lock() {
    check_for_hierarchy_violation();
    if (!internal_mutex_.try_lock()) {
      return false;
    }
    update_hierarchy_value();
    return true;
  }

private:
  void check_for_hierarchy_violation() {
    if (this_thread_hierarchy_value_ <= hierarchy_level_) {
      throw std::logic_error("Mutex hierarchy violated! Cannot lock level " +
                             std::to_string(hierarchy_level_) +
                             " when holding level " +
                             std::to_string(this_thread_hierarchy_value_));
    }
  }

  void update_hierarchy_value() {
    previous_hierarchy_level_ = this_thread_hierarchy_value_;
    this_thread_hierarchy_value_ = hierarchy_level_;
  }

  std::mutex internal_mutex_;
  unsigned long const hierarchy_level_;
  unsigned long previous_hierarchy_level_;

  // Each thread has its own hierarchy value
  // Initialize to max so any mutex can be locked first
  inline static thread_local unsigned long this_thread_hierarchy_value_ = ULONG_MAX;
};

// High level = lock first
hierarchical_mutex high_level_mutex(10000);
hierarchical_mutex mid_level_mutex(5000);
hierarchical_mutex low_level_mutex(1000);

// ✅ CORRECT: High → Low order
void do_work_correct() {
  std::lock_guard<hierarchical_mutex> high(high_level_mutex);
  std::cout << "[Thread " << std::this_thread::get_id() << "] Locked 10000\n";

  std::lock_guard<hierarchical_mutex> mid(mid_level_mutex);
  std::cout << "[Thread " << std::this_thread::get_id() << "] Locked 5000\n";

  std::lock_guard<hierarchical_mutex> low(low_level_mutex);
  std::cout << "[Thread " << std::this_thread::get_id() << "] Locked 1000\n";

  std::cout << "[Thread " << std::this_thread::get_id() << "] Doing work...\n";
}

// ❌ WRONG: Violates hierarchy
void do_work_wrong() {
  std::lock_guard<hierarchical_mutex> low(low_level_mutex);
  std::cout << "[Thread " << std::this_thread::get_id() << "] Locked 1000\n";

  // This will throw! Can't lock 10000 when holding 1000
  std::lock_guard<hierarchical_mutex> high(high_level_mutex);
  std::cout << "[Thread " << std::this_thread::get_id() << "] Locked 10000\n";
}

int main() {
  std::cout << "=== Correct order ===\n";
  try {
    std::thread t1(do_work_correct);
    std::thread t2(do_work_correct);
    t1.join();
    t2.join();
    std::cout << "✅ Success!\n\n";
  } catch (const std::exception &e) {
    std::cout << "❌ Error: " << e.what() << "\n\n";
  }

  std::cout << "=== Wrong order ===\n";
  try {
    std::thread t3(do_work_wrong);
    t3.join();
    std::cout << "✅ Success!\n";
  } catch (...) {
    // Exception thrown in thread, need to catch there
  }

  // Run in main thread to see exception
  try {
    do_work_wrong();
  } catch (const std::exception &e) {
    std::cout << "❌ Caught: " << e.what() << "\n";
  }

  return 0;
}