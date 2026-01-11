#include <chrono>
#include <iostream>
#include <thread>

void worker() {
  std::this_thread::sleep_for(std::chrono::seconds(3));
  std::cout << "Worker finished!" << std::endl; // May never print!
}

class background_task {
public:
  void operator()() { std::cout << "background_task called" << std::endl; }
};

int main() {
  // std::thread t1(worker);
  std::thread t1{background_task()};
  t1.join();

  std::cout << "Main exiting..." << std::endl;
  return 0;
}