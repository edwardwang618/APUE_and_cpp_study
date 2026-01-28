#include <iostream>
#include <thread>
#include <vector>
#include "shared_lock_1.h"

SharedMutex rw_mutex;
int shared_data = 0;

// 读者线程
void reader(int id) {
  SharedLock<SharedMutex> lock(rw_mutex); // 自动获取读锁
  std::cout << "Reader " << id << " reads: " << shared_data << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  // 自动释放读锁（析构时）
}

// 写者线程
void writer(int id) {
  UniqueLock<SharedMutex> lock(rw_mutex); // 自动获取写锁
  ++shared_data;
  std::cout << "Writer " << id << " writes: " << shared_data << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  // 自动释放写锁（析构时）
}

int main() {
  std::vector<std::thread> threads;

  // 启动多个读者和写者
  for (int i = 0; i < 3; i++) {
    threads.emplace_back(reader, i);
  }
  threads.emplace_back(writer, 0);
  for (int i = 3; i < 6; i++) {
    threads.emplace_back(reader, i);
  }
  threads.emplace_back(writer, 1);

  for (auto &t : threads) {
    t.join();
  }

  return 0;
}
