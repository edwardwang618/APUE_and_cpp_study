#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <thread>

std::mutex mu;
std::condition_variable cv;
bool flag = false;

int main() {
  std::thread t([&flag = flag] {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::lock_guard<std::mutex> lk(mu);
    std::cout << "Hello ";
    fflush(stdout);
    flag = true;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    cv.notify_one();
  });

  std::unique_lock<std::mutex> lk(mu);
  cv.wait(lk, [] { return flag; });
  std::cout << "World" << std::endl;

  t.join();
}