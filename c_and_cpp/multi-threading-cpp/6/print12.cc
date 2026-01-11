#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

std::mutex mu;
std::condition_variable cv1, cv2;
int num = 1;

int main() {
  std::thread t1([]() {
    while (true) {
      std::unique_lock<std::mutex> lk(mu);
      cv1.wait(lk, []() { return num == 1; });
      num++;
      std::cout << "thread t1 print 1" << std::endl;
      cv2.notify_all();
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  });

  std::thread t2([]() {
    while (true) {
      std::unique_lock<std::mutex> lk(mu);
      cv2.wait(lk, []() { return num == 2; });
      num--;
      std::cout << "thread t2 print 2" << std::endl;
      cv1.notify_all();
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  });

  t1.join();
  t2.join();
}