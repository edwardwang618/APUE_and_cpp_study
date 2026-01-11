#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

std::mutex mtx1;
int shared_data = 100;

void use_lock() {
  while (true) {
    {
      std::lock_guard<std::mutex> lk_guard(mtx1);
      shared_data--;
      std::cout << "current thread is " << std::this_thread::get_id()
                << std::endl;
      std::cout << "shared data is " << shared_data << std::endl;
    }

    // std::this_thread::sleep_for(std::chrono::microseconds(10));

    if (shared_data <= 0)
      break;
  }
}

void test_lock() {
  std::thread t1(use_lock);
  std::thread t2(use_lock);

  t1.join();
  t2.join();
}

int main() {
  test_lock();
  std::cout << "Final shared_data: " << shared_data << std::endl;
  return 0;
}