#include <chrono>
#include <iostream>
#include <thread>

int main() {
  std::string hellostr = "Hello";
  std::thread t1{
      [](std::string s) { std::cout << "str is: " << s << std::endl; },
      hellostr};

  t1.join();

  std::cout << "Main exiting..." << std::endl;
  return 0;
}