#include <iostream>
#include <mutex>
#include <thread>

std::mutex mutex1;
std::mutex mutex2;

void thread1() {
  std::lock_guard<std::mutex> lock1(mutex1); // Lock mutex1 first
  std::cout << "Thread 1: locked mutex1\n";

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  std::lock_guard<std::mutex> lock2(mutex2); // Then try mutex2
  std::cout << "Thread 1: locked mutex2\n";
}

void thread2() {
  std::lock_guard<std::mutex> lock2(mutex2); // Lock mutex2 first
  std::cout << "Thread 2: locked mutex2\n";

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  std::lock_guard<std::mutex> lock1(mutex1); // Then try mutex1
  std::cout << "Thread 2: locked mutex1\n";
}

int main() {
  std::thread t1(thread1);
  std::thread t2(thread2);

  t1.join();
  t2.join();

  std::cout << "Done\n"; // Never reached!
}