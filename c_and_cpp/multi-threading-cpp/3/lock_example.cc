#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

std::mutex mtx1;
int shared_data = 100;

void use_lock() {
  for (int i = 0; i < 5; ++i) {
    {
      std::lock_guard<std::mutex> lock(mtx1); // Auto locks
      shared_data++;
      std::cout << "thread " << std::this_thread::get_id()
                << ", data = " << shared_data << std::endl;
    } // Auto unlocks here

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

int main() {
  std::thread t1(use_lock);
  std::thread t2(use_lock);
  std::thread t3(use_lock);

  t1.join();
  t2.join();
  t3.join();

  std::cout << "Final: " << shared_data << std::endl;
  return 0;
}