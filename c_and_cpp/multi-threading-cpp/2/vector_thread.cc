#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

void param_function(unsigned i) {
  std::cout << "Thread " << i << " started" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "Thread " << i << " finished" << std::endl;
}

void use_vector() {
  std::vector<std::thread> threads;

  for (unsigned i = 0; i < 5; ++i) {
    // Option 1: push_back with move
    // auto t = std::thread(param_function, i);
    // threads.push_back(std::move(t));

    // Option 2: emplace_back (simpler)
    threads.emplace_back(param_function, i);
  }

  // All threads are ALREADY running!
  // join() just waits for them to finish
  for (auto &entry : threads) {
    std::cout << "Joining a thread..." << std::endl;
    entry.join();
  }
}

int main() {
  use_vector();
  return 0;
}