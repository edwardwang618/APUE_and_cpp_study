#include <functional>
#include <iostream>
#include <memory>
#include <queue>

class TaskQueue {
  std::queue<std::move_only_function<void()>> tasks_;

public:
  void post(std::move_only_function<void()> task) {
    tasks_.push(std::move(task));
  }

  void run_one() {
    if (tasks_.empty())
      return;
    auto task = std::move(tasks_.front());
    tasks_.pop();
    task();
  }
};

int main() {
  TaskQueue q;
  auto resource = std::make_unique<int>(100);

  q.post([r = std::move(resource)]() {
    std::cout << "Got resource: " << *r << '\n';
  });

  q.run_one();
}