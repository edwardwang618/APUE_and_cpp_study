#include <chrono>
#include <iostream>
#include <string>
#include <thread>

void thead_work1(std::string str) {
  std::cout << "str is " << str << std::endl;
}

int main() {
  // 通过()初始化并启动一个线程
  std::string hellostr = "hello";
  std::thread t1(thead_work1, hellostr);

  // t1.join(); // Wait for thread to finish
  std::this_thread::sleep_for(std::chrono::seconds(1));
  return 0;
}