#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

class Worker {
public:
  Worker(std::shared_ptr<int> ptr) : data(ptr) {}

  void operator()() {
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Safe! shared_ptr keeps data alive
    std::cout << "Worker accessing data: " << *data << std::endl;
    *data = 100;
  }

private:
  std::shared_ptr<int> data; // Shared ownership
};

void good_example() {
  auto local_var = std::make_shared<int>(42);

  std::cout << "Before thread, use_count: " << local_var.use_count()
            << std::endl;

  std::thread t{Worker(local_var)};

  std::cout << "After thread created, use_count: " << local_var.use_count()
            << std::endl;

  t.detach();
} // local_var destroyed here, but Worker still holds a copy of shared_ptr

int main() {
  good_example();

  std::this_thread::sleep_for(std::chrono::seconds(2));

  std::cout << "Main exiting..." << std::endl;
  return 0;
}