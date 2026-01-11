#include <future>
#include <iostream>

int main() {
  std::packaged_task<int()> task([]() { return 42; });

  // Get future BEFORE move
  auto future = task.get_future();

  // Move task into lambda
  auto lambda = [t = std::move(task)]() mutable { t(); };
  // auto lambda = [t = std::move(task)]() { t(); };

  // Original task is now empty
  // task.get_future();  // would throw!

  // Run the lambda (executes moved task)
  lambda();

  // Future still works!
  std::cout << future.get() << std::endl; // prints 42

  return 0;
}