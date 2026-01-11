#include <future>
#include <iostream>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

std::string loadData(std::string key) {
  std::cout << "Loading " << key << "..." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(2));
  return "Data for " + key;
}

class DataCache {
  std::map<std::string, std::shared_future<std::string>> cache_;
  std::mutex mu_;

public:
  std::shared_future<std::string> get(std::string key) {
    std::lock_guard lk(mu_);

    if (cache_.count(key)) {
      std::cout << "Cache hit: " << key << std::endl;
      return cache_[key]; // copy out, original stays in cache
    }

    std::cout << "Cache miss: " << key << std::endl;
    auto sf = std::async(std::launch::async, loadData, key).share();
    cache_[key] = sf;
    return sf;
  }
};

int main() {
  DataCache cache;

  // 5 threads request same key
  std::vector<std::thread> threads;
  for (int i = 0; i < 5; i++) {
    threads.emplace_back([&cache, i]() {
      auto fut = cache.get("user123");
      std::string data = fut.get();
      std::cout << "Thread " << i << " got: " << data << std::endl;
    });
  }

  for (auto &t : threads) {
    t.join();
  }

  return 0;
}