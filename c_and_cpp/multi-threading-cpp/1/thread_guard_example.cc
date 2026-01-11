#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

class ThreadGuard {
public:
  explicit ThreadGuard(std::thread &t) : thread_(t) {}

  ~ThreadGuard() {
    std::cout << "ThreadGuard destructor called" << std::endl;
    if (thread_.joinable()) {
      std::cout << "Joining thread..." << std::endl;
      thread_.join();
      std::cout << "Thread joined!" << std::endl;
    }
  }

  // Prevent copying
  ThreadGuard(const ThreadGuard &) = delete;
  ThreadGuard &operator=(const ThreadGuard &) = delete;

private:
  std::thread &thread_;
};

void worker() {
  std::cout << "Worker started" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(2));
  std::cout << "Worker finished" << std::endl;
}

// Example 1: Normal exit
void normal_exit() {
  std::cout << "\n=== Normal Exit Example ===" << std::endl;

  std::thread t(worker);
  ThreadGuard guard(t);

  std::cout << "Main doing work..." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "Main work done" << std::endl;

  // guard destructor automatically calls join()
}

// Example 2: Exception thrown
void exception_exit() {
  std::cout << "\n=== Exception Exit Example ===" << std::endl;

  std::thread t(worker);
  ThreadGuard guard(t);

  std::cout << "Main doing work..." << std::endl;
  throw std::runtime_error("Something went wrong!");

  // Never reached, but guard destructor still calls join()!
}

// Example 3: Without ThreadGuard (BAD!)
void bad_example() {
  std::cout << "\n=== Bad Example (No Guard) ===" << std::endl;

  std::thread t(worker);

  throw std::runtime_error("Oops!");

  t.join(); // Never reached! Program crashes with std::terminate()
}

int main() {
  // // Example 1: Normal exit
  // normal_exit();

  // Example 2: Exception handled safely
  try {
    exception_exit();
  } catch (std::exception &e) {
    std::cout << "Caught exception: " << e.what() << std::endl;
  }

  // // Example 3: Uncomment to see crash
  // // bad_example();

  std::cout << "\nAll done!" << std::endl;
  return 0;
}