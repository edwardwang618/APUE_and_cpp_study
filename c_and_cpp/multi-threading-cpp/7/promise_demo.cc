#include <chrono>
#include <future>
#include <iostream>
#include <thread>

void basic_demo() {
  std::cout << "=== Basic Demo ===" << std::endl;

  // Create promise
  std::promise<int> prom;

  // Get future before moving promise
  std::future<int> fut = prom.get_future();

  // Thread that will set the value
  std::thread t(
      [](std::promise<int> p) {
        std::cout << "Worker: doing work..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        p.set_value(42);
        std::cout << "Worker: value set!" << std::endl;
      },
      std::move(prom));

  // Main thread waits for value
  std::cout << "Main: waiting for value..." << std::endl;
  int result = fut.get();
  std::cout << "Main: got value = " << result << std::endl;

  t.join();
}

void exception_demo() {
  std::cout << "\n=== Exception Demo ===" << std::endl;

  std::promise<int> prom;
  std::future<int> fut = prom.get_future();

  std::thread t(
      [](std::promise<int> p) {
        try {
          // Simulate failure
          throw std::runtime_error("Something went wrong!");
        } catch (...) {
          // Pass exception to future
          p.set_exception(std::current_exception());
        }
      },
      std::move(prom));

  try {
    int result = fut.get(); // throws here
    std::cout << "Result: " << result << std::endl;
  } catch (const std::exception &e) {
    std::cout << "Caught: " << e.what() << std::endl;
  }

  t.join();
}

void void_promise_demo() {
  std::cout << "\n=== Void Promise (Signaling) ===" << std::endl;

  std::promise<void> ready;
  std::future<void> ready_fut = ready.get_future();

  std::thread t(
      [](std::future<void> f) {
        std::cout << "Worker: waiting for signal..." << std::endl;
        f.get(); // blocks until signaled
        std::cout << "Worker: got signal, starting work!" << std::endl;
      },
      std::move(ready_fut));

  std::cout << "Main: preparing..." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));

  std::cout << "Main: sending signal" << std::endl;
  ready.set_value(); // signal the thread

  t.join();
}

void multiple_waiters_demo() {
  std::cout << "\n=== Multiple Waiters (shared_future) ===" << std::endl;

  std::promise<int> prom;
  std::shared_future<int> fut = prom.get_future().share();

  // Multiple threads can wait on shared_future
  std::thread t1(
      [](std::shared_future<int> f) {
        std::cout << "Thread 1: got " << f.get() << std::endl;
      },
      fut);

  std::thread t2(
      [](std::shared_future<int> f) {
        std::cout << "Thread 2: got " << f.get() << std::endl;
      },
      fut);

  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  prom.set_value(100);

  t1.join();
  t2.join();
}

int main() {
  basic_demo();
  exception_demo();
  void_promise_demo();
  multiple_waiters_demo();
  return 0;
}