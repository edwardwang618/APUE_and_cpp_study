#include <chrono>
#include <iostream>
#include <thread>

void some_function() {
  std::cout << "Working..." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(2));
  std::cout << "Done!" << std::endl;
}

void with_thread() {
  std::thread t(some_function);
  // Must manually join or detach!
  t.join();
}

void with_jthread() {
  std::jthread t(some_function);
  // No need to join!
  // Destructor automatically joins
}

int main() {
  with_jthread();
  std::cout << "After jthread" << std::endl;
  return 0;
}