#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

// 互斥锁，保护共享队列的访问
std::mutex mu;

// 两个条件变量分别用于不同的等待条件
// cv_prod: 生产者等待"队列未满"
// cv_cons: 消费者等待"队列非空"
std::condition_variable cv_prod, cv_cons;

// 共享的有界队列
std::queue<int> q;
constexpr int cap = 10;

// 原子变量，用于优雅退出
// 使用 atomic 是因为信号处理函数和其他线程都会访问它
std::atomic<bool> shutdown_flag{false};

// 信号处理函数
// 注意：信号处理函数中只能安全地使用 async-signal-safe 的操作
// 写入 atomic<bool> 和 write() 系统调用是安全的
void signal_handler(int sig) {
  // 使用 write 而不是 cout，因为 cout 不是 async-signal-safe
  const char msg[] = "\nReceived Ctrl+C, shutting down...\n";
  write(STDOUT_FILENO, msg, sizeof(msg) - 1);

  // 设置退出标志，使用 relaxed 内存序即可
  shutdown_flag.store(true, std::memory_order_relaxed);

  // 必须唤醒所有等待的线程，让它们检查 shutdown_flag
  // 这里用 notify_all 而不是 notify_one
  cv_prod.notify_all();
  cv_cons.notify_all();
}

void producer() {
  while (true) {
    std::unique_lock<std::mutex> lk(mu);

    // 等待条件：队列未满 或者 收到退出信号
    // 必须把 shutdown_flag 加入条件，否则线程可能永远阻塞
    cv_prod.wait(lk, [] {
      return q.size() < cap || shutdown_flag.load(std::memory_order_relaxed);
    });

    // 检查是否需要退出
    if (shutdown_flag.load(std::memory_order_relaxed)) {
      std::cout << std::this_thread::get_id() << " Producer exiting"
                << std::endl;
      return;
    }

    int x = rand();
    std::cout << std::this_thread::get_id() << " Producing " << x << std::endl;
    q.push(x);

    // 通知一个等待的消费者：队列非空了
    cv_cons.notify_one();
  }
}

void consumer() {
  while (true) {
    std::unique_lock<std::mutex> lk(mu);

    // 等待条件：队列非空 或者 收到退出信号
    cv_cons.wait(lk, [] {
      return !q.empty() || shutdown_flag.load(std::memory_order_relaxed);
    });

    // 检查是否需要退出
    // 注意：即使收到退出信号，如果队列还有数据，可以选择继续消费
    // 这里选择直接退出，根据业务需求可以调整
    if (shutdown_flag.load(std::memory_order_relaxed)) {
      std::cout << std::this_thread::get_id() << " Consumer exiting"
                << std::endl;
      return;
    }

    int x = q.front();
    q.pop();
    std::cout << std::this_thread::get_id() << " Consuming " << x << std::endl;

    // 通知一个等待的生产者：队列有空位了
    cv_prod.notify_one();
  }
}

int main() {
  // 注册信号处理函数
  std::signal(SIGINT, signal_handler);

  std::vector<std::thread> ts;
  int cnt = 5;

  for (int i = 0; i < cnt; i++) {
    ts.emplace_back(producer);
    ts.emplace_back(consumer);
  }

  // 等待所有线程结束
  for (auto &t : ts) {
    t.join();
  }

  std::cout << "All threads finished, goodbye!" << std::endl;
  return 0;
}