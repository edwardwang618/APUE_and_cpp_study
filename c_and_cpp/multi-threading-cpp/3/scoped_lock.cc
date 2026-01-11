#include <iostream>
#include <mutex>
#include <thread>

std::mutex mutex1;
std::mutex mutex2;

void thread1() {
  std::scoped_lock lock(mutex1, mutex2); // Order doesn't matter!
  std::cout << "Thread 1: locked both\n";
}

void thread2() {
  std::scoped_lock lock(mutex2, mutex1); // Different order - still safe!
  std::cout << "Thread 2: locked both\n";
}

int main() {
  std::thread t1(thread1);
  std::thread t2(thread2);

  t1.join();
  t2.join();

  std::cout << "Done\n"; // Always reached!
}