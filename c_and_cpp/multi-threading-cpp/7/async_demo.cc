#include <chrono>
#include <future>
#include <iostream>
#include <string>
#include <thread>

// Simulate a slow database query
std::string fetchDataFromDB(std::string query) {
  std::cout << "DB query started on thread " << std::this_thread::get_id()
            << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(3));
  return "Data: " + query;
}

// Simulate another slow task
int compute(int x) {
  std::cout << "Compute started on thread " << std::this_thread::get_id()
            << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(2));
  return x * x;
}

void demo_async() {
  std::cout << "Main thread: " << std::this_thread::get_id() << std::endl;

  auto start = std::chrono::steady_clock::now();

  // Launch async tasks
  std::future<std::string> dbFuture =
      std::async(std::launch::async, fetchDataFromDB, "SELECT * FROM users");

  std::future<int> computeFuture = std::async(std::launch::async, compute, 42);

  // Do something in main thread
  std::cout << "Main thread doing other work..." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Get results (blocks if not ready)
  std::string dbResult = dbFuture.get();
  int computeResult = computeFuture.get();

  auto end = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start);

  std::cout << "\nResults:" << std::endl;
  std::cout << "DB: " << dbResult << std::endl;
  std::cout << "Compute: " << computeResult << std::endl;
  std::cout << "Total time: " << elapsed.count() << "s" << std::endl;
  std::cout << "(Sequential would be 6s)" << std::endl;
}

void demo_launch_policies() {
  std::cout << "\n--- Launch Policies ---\n" << std::endl;

  // std::launch::async - guaranteed new thread
  auto f1 = std::async(std::launch::async, []() {
    std::cout << "async: runs on thread " << std::this_thread::get_id()
              << std::endl;
    return 1;
  });

  // std::launch::deferred - lazy, runs when .get() called
  auto f2 = std::async(std::launch::deferred, []() {
    std::cout << "deferred: runs on thread " << std::this_thread::get_id()
              << std::endl;
    return 2;
  });

  // default - implementation chooses
  auto f3 = std::async([]() {
    std::cout << "default: runs on thread " << std::this_thread::get_id()
              << std::endl;
    return 3;
  });

  std::cout << "Main thread: " << std::this_thread::get_id() << std::endl;
  std::cout << "Before .get() calls..." << std::endl;

  std::cout << "f1 result: " << f1.get() << std::endl;
  std::cout << "f2 result: " << f2.get() << std::endl; // runs NOW
  std::cout << "f3 result: " << f3.get() << std::endl;
}

void demo_exception_handling() {
  std::cout << "\n--- Exception Handling ---\n" << std::endl;

  auto f = std::async(std::launch::async, []() -> int {
    throw std::runtime_error("Something went wrong!");
    return 42;
  });

  try {
    int result = f.get(); // exception rethrown here
    std::cout << "Result: " << result << std::endl;
  } catch (const std::exception &e) {
    std::cout << "Caught exception: " << e.what() << std::endl;
  }
}

int main() {
  demo_async();
  demo_launch_policies();
  demo_exception_handling();
  return 0;
}