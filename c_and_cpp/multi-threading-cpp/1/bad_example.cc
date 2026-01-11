#include <chrono>
#include <iostream>
#include <thread>

class Worker {
public:
  Worker(int &ref) : data(ref) {}

  void operator()() {
    // Simulate some work
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // DANGER: data reference may be invalid here!
    std::cout << "Worker accessing data: " << data << std::endl;
    data = 100; // Writing to freed memory!
  }

private:
  int &data; // Holds a reference to local variable
};

void bad_example() {
  int local_var = 42;

  std::thread t{Worker(local_var)};
  t.detach(); // Thread runs independently

  // Function returns, local_var is destroyed
  // But the detached thread still holds a reference to it!
}

int main() {
  bad_example();

  // Give detached thread time to run
  std::this_thread::sleep_for(std::chrono::seconds(2));

  std::cout << "Main exiting..." << std::endl;
  return 0;
}