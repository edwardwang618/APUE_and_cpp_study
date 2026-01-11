#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include "my_shared.h"

class DataStore {
private:
  int data_ = 0;
  mutable SharedMutex mtx_;

public:
  int read() const {
    SharedLock<SharedMutex> lock(mtx_);
    std::cout << "[Thread " << std::this_thread::get_id()
              << "] Reading: " << data_ << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return data_;
  }

  void write(int value) {
    UniqueLock<SharedMutex> lock(mtx_);
    std::cout << "[Thread " << std::this_thread::get_id()
              << "] Writing: " << value << "\n";
    data_ = value;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
};

int main() {
  DataStore store;

  std::vector<std::thread> threads;

  // 3 readers
  for (int i = 0; i < 3; ++i) {
    threads.emplace_back([&store]() {
      for (int j = 0; j < 3; ++j) {
        store.read();
      }
    });
  }

  // 1 writer
  threads.emplace_back([&store]() {
    for (int j = 1; j <= 3; ++j) {
      store.write(j * 100);
    }
  });

  for (auto &t : threads) {
    t.join();
  }

  return 0;
}