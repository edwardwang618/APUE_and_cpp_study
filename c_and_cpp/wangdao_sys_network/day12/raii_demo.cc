// raii_demo.cpp - Using the RAII wrapper
#include "shared_memory.hpp"
#include <atomic>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

struct Counter {
  std::atomic<int> value{0};
  std::atomic<bool> done{false};
};

int main() {
  auto shm = SharedMemory<Counter>::create("/counter_shm");

  pid_t pid = fork();

  if (pid == 0) {
    // Child process
    auto child_shm = SharedMemory<Counter>::open("/counter_shm");

    for (int i = 0; i < 100000; ++i) {
      child_shm->value.fetch_add(1);
    }

    std::cout << "Child: Done incrementing\n";
    _exit(0);
  }

  // Parent process
  for (int i = 0; i < 100000; ++i) {
    shm->value.fetch_add(1);
  }
  std::cout << "Parent: Done incrementing\n";

  wait(nullptr);

  std::cout << "Final value: " << shm->value.load() << " (expected: 200000)\n";

  return 0;
}