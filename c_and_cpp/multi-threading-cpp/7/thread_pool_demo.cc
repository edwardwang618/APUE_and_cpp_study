#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
public:
  using Task = std::packaged_task<void()>;

  static ThreadPool &instance() {
    static ThreadPool ins;
    return ins;
  }

  ThreadPool(unsigned int num = 5) : stop_(false) {
    if (num < 1)
      thread_num_ = 1;
    else
      thread_num_ = num;
    start();
  }

  ~ThreadPool() { stop(); }

  template <typename F, typename... Args>
  auto commit(F &&f, Args &&...args) -> std::future<decltype(f(args...))> {
    using RetType = decltype(f(args...));

    auto task = std::make_shared<std::packaged_task<RetType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<RetType> future = task->get_future();

    {
      std::lock_guard<std::mutex> lk(mu_);
      tasks_.emplace([task]() { (*task)(); });
    }

    cv_.notify_one();
    return future;
  }

  int idleThreadCount() { return idle_thread_num_; }

private:
  void start() {
    for (unsigned int i = 0; i < thread_num_; i++) {
      threads_.emplace_back([this]() {
        while (true) {
          Task task;

          {
            std::unique_lock<std::mutex> lk(mu_);

            idle_thread_num_++;

            cv_.wait(lk, [this]() { return stop_ || !tasks_.empty(); });

            idle_thread_num_--;

            if (stop_ && tasks_.empty()) {
              return;
            }

            task = std::move(tasks_.front());
            tasks_.pop();
          }

          task();
        }
      });
    }
  }

  void stop() {
    {
      std::lock_guard<std::mutex> lk(mu_);
      stop_ = true;
    }

    cv_.notify_all();

    for (auto &t : threads_) {
      if (t.joinable()) {
        t.join();
      }
    }
  }

  unsigned int thread_num_;
  std::atomic<int> idle_thread_num_{0};
  std::atomic<bool> stop_;
  std::mutex mu_;
  std::condition_variable cv_;
  std::queue<Task> tasks_;
  std::vector<std::thread> threads_;
};

// ============ Demo ============

int add(int a, int b) {
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  std::cout << "add(" << a << ", " << b << ") on thread "
            << std::this_thread::get_id() << std::endl;
  return a + b;
}

std::string concat(std::string a, std::string b) {
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  return a + b;
}

void printMessage(std::string msg) {
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  std::cout << "Message: " << msg << std::endl;
}

int main() {
  std::cout << "=== Thread Pool Demo ===" << std::endl;

  ThreadPool pool(3); // 3 worker threads

  // Submit tasks with return values
  std::future<int> f1 = pool.commit(add, 1, 2);
  std::future<int> f2 = pool.commit(add, 10, 20);
  std::future<std::string> f3 = pool.commit(concat, "Hello, ", "World!");

  // Submit void task
  pool.commit(printMessage, "Fire and forget");

  // Submit lambda
  std::future<int> f4 = pool.commit([]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return 42;
  });

  // Get results
  std::cout << "Result 1: " << f1.get() << std::endl;
  std::cout << "Result 2: " << f2.get() << std::endl;
  std::cout << "Result 3: " << f3.get() << std::endl;
  std::cout << "Result 4: " << f4.get() << std::endl;

  std::cout << "Idle threads: " << pool.idleThreadCount() << std::endl;

  return 0;
}