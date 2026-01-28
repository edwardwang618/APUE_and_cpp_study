#include "shared_lock_2.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

SharedMutexWriterPriority rw_mutex;
int shared_data = 0;

void reader(int id) {
  std::cout << "Reader " << id << " waiting..." << std::endl;

  rw_mutex.lock_shared();
  std::cout << "Reader " << id << " reading: " << shared_data << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  rw_mutex.unlock_shared();

  std::cout << "Reader " << id << " done" << std::endl;
}

void writer(int id) {
  std::cout << "Writer " << id << " waiting..." << std::endl;

  rw_mutex.lock();
  ++shared_data;
  std::cout << "Writer " << id << " writing: " << shared_data << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  rw_mutex.unlock();

  std::cout << "Writer " << id << " done" << std::endl;
}

int main() {
  std::vector<std::thread> threads;

  // 先启动读者
  threads.emplace_back(reader, 1);
  threads.emplace_back(reader, 2);

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // 写者请求
  threads.emplace_back(writer, 1);

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // 更多读者请求 (会被阻塞，因为写者在等待)
  threads.emplace_back(reader, 3);
  threads.emplace_back(reader, 4);

  for (auto &t : threads) {
    t.join();
  }

  return 0;
}
