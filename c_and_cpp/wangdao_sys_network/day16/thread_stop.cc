#include <chrono>
#include <iostream>
#include <thread>

void threadFunc(std::stop_token stoken, int id, std::string name) {
  while (!stoken.stop_requested()) {
    std::cout << "Thread " << id << " (" << name << ") running" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

int main() {
  // stop_token is auto-passed, then your args
  std::jthread t(threadFunc, 42, "worker");

  std::this_thread::sleep_for(std::chrono::seconds(3));
  t.request_stop();

  return 0;
}