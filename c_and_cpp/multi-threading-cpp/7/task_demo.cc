#include <chrono>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

int my_task(int x) {
  std::cout << "Task running on thread " << std::this_thread::get_id()
            << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(2));
  return x * x;
}

void basic_demo() {
  std::cout << "=== Basic Demo ===" << std::endl;
  std::cout << "Main thread: " << std::this_thread::get_id() << std::endl;

  // Create packaged_task wrapping a function
  std::packaged_task<int(int)> task(my_task);

  // Get future before moving task
  std::future<int> result = task.get_future();

  // Move task to thread and run
  std::thread t(std::move(task), 7);
  t.detach();

  // Wait for result
  int value = result.get();
  std::cout << "Result: " << value << std::endl;
}

void lambda_demo() {
  std::cout << "\n=== Lambda Demo ===" << std::endl;

  std::packaged_task<std::string(std::string, int)> task(
      [](std::string name, int count) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::string result;
        for (int i = 0; i < count; ++i) {
          result += name + " ";
        }
        return result;
      });

  auto future = task.get_future();

  std::thread t(std::move(task), "Hello", 3);
  t.join();

  std::cout << "Result: " << future.get() << std::endl;
}

void run_on_main_thread() {
  std::cout << "\n=== Run on Main Thread ===" << std::endl;

  std::packaged_task<int()> task([]() { return 42; });

  auto future = task.get_future();

  // Call directly, no thread
  task();

  std::cout << "Result: " << future.get() << std::endl;
}

void task_queue_demo() {
  std::cout << "\n=== Task Queue Demo ===" << std::endl;

  std::vector<std::packaged_task<int()>> tasks;
  std::vector<std::future<int>> futures;

  // Create tasks
  for (int i = 1; i <= 3; ++i) {
    std::packaged_task<int()> task([i]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(100 * i));
      return i * 10;
    });
    futures.push_back(task.get_future());
    tasks.push_back(std::move(task));
  }

  // Run tasks on separate threads
  std::vector<std::thread> threads;
  for (auto &task : tasks) {
    threads.emplace_back(std::move(task));
  }

  // Collect results
  for (size_t i = 0; i < futures.size(); ++i) {
    std::cout << "Task " << i + 1 << " result: " << futures[i].get()
              << std::endl;
  }

  for (auto &t : threads) {
    t.join();
  }
}

int main() {
  basic_demo();
  // lambda_demo();
  // run_on_main_thread();
  // task_queue_demo();
  return 0;
}