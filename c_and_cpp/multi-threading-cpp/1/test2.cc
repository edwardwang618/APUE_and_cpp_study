#include <chrono>
#include <iostream>
#include <thread>

void worker() {
  std::this_thread::sleep_for(std::chrono::seconds(3));
  std::cout << "Worker finished!" << std::endl; // May never print!
}

int main() {
  std::thread t1(worker);
  t1.detach();

  std::cout << "Main exiting..." << std::endl;
  return 0; // Main exits immediately, detached thread gets killed
}