#include <iostream>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

class ThreadSafeCache {
  mutable std::shared_mutex mutex_;
  std::map<std::string, int> cache_;

public:
  int read(const std::string &key) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto it = cache_.find(key);
    return (it != cache_.end()) ? it->second : -1;
  }

  void write(const std::string &key, int value) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    cache_[key] = value;
  }
};

int main() {
  ThreadSafeCache cache;

  // Writer thread
  std::thread writer([&cache]() {
    for (int i = 0; i < 10; ++i) {
      cache.write("key" + std::to_string(i), i * 100);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  });

  // Multiple reader threads
  std::vector<std::thread> readers;
  for (int r = 0; r < 3; ++r) {
    readers.emplace_back([&cache, r]() {
      for (int i = 0; i < 10; ++i) {
        int val = cache.read("key" + std::to_string(i));
        std::cout << "Reader " << r << " got key" << i << " = " << val << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
      }
    });
  }

  writer.join();
  for (auto &t : readers) {
    t.join();
  }

  return 0;
}