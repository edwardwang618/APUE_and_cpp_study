#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <thread>

template <typename T, size_t Cap> class SPSCQueue {
  alignas(64) std::atomic<size_t> head{0};
  alignas(64) std::atomic<size_t> tail{0};
  alignas(64) size_t cached_head{0};
  alignas(64) size_t cached_tail{0};
  alignas(64) std::array<T, Cap + 1> buffer;

  static constexpr size_t next_pos(size_t pos) { return (pos + 1) % (Cap + 1); }

public:
  bool push(const T &val) {
    size_t pos = tail.load(std::memory_order_relaxed);
    size_t next = next_pos(pos);

    if (next == cached_head) {
      cached_head = head.load(std::memory_order_acquire);
      if (next == cached_head)
        return false;
    }

    buffer[pos] = val;
    tail.store(next, std::memory_order_release);
    return true;
  }

  bool pop(T &val) {
    size_t pos = head.load(std::memory_order_relaxed);

    if (pos == cached_tail) {
      cached_tail = tail.load(std::memory_order_acquire);
      if (pos == cached_tail)
        return false;
    }

    val = std::move(buffer[pos]);
    head.store(next_pos(pos), std::memory_order_release);
    return true;
  }

  size_t size() const {
    size_t h = head.load(std::memory_order_acquire);
    size_t t = tail.load(std::memory_order_acquire);
    return (t - h + Cap + 1) % (Cap + 1);
  }
};

// ============== 示例 1: 基础用法 ==============

void basic_example() {
  std::cout << "=== 基础用法示例 ===" << std::endl;

  SPSCQueue<int, 8> queue;
  std::atomic<bool> done{false};

  // 生产者线程（Producer Thread）
  std::thread producer([&]() {
    for (int i = 1; i <= 20; ++i) {
      // 自旋等待直到 push 成功
      while (!queue.push(i)) {
        std::this_thread::yield(); // 让出 CPU
      }
      std::cout << "[Producer] Pushed: " << i << std::endl;
    }
    done.store(true, std::memory_order_release);
  });

  // 消费者线程（Consumer Thread）
  std::thread consumer([&]() {
    int val;
    int count = 0;

    while (count < 20) {
      if (queue.pop(val)) {
        std::cout << "[Consumer] Popped: " << val << std::endl;
        ++count;
      } else {
        std::this_thread::yield();
      }
    }
  });

  producer.join();
  consumer.join();

  std::cout << std::endl;
}

// ============== 示例 2: 性能测试 ==============

void benchmark_example() {
  std::cout << "=== 性能测试示例 ===" << std::endl;

  constexpr size_t NUM_ITEMS = 10'000'000;
  SPSCQueue<uint64_t, 1024> queue;
  std::atomic<bool> done{false};

  auto start = std::chrono::high_resolution_clock::now();

  // 生产者：尽可能快地推送数据
  std::thread producer([&]() {
    for (uint64_t i = 0; i < NUM_ITEMS; ++i) {
      while (!queue.push(i)) {
        // 忙等待（Busy Wait）
      }
    }
    done.store(true, std::memory_order_release);
  });

  // 消费者：尽可能快地消费数据
  std::thread consumer([&]() {
    uint64_t val;
    uint64_t count = 0;
    uint64_t sum = 0;

    while (count < NUM_ITEMS) {
      if (queue.pop(val)) {
        sum += val;
        ++count;
      }
    }

    // 验证正确性：sum 应该等于 0 + 1 + ... + (N-1)
    uint64_t expected = (NUM_ITEMS - 1) * NUM_ITEMS / 2;
    if (sum == expected) {
      std::cout << "[Consumer] 校验通过! Sum = " << sum << std::endl;
    } else {
      std::cout << "[Consumer] 校验失败! Got " << sum << ", expected "
                << expected << std::endl;
    }
  });

  producer.join();
  consumer.join();

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  double ops_per_sec = static_cast<double>(NUM_ITEMS) / duration.count() * 1000;
  std::cout << "传输 " << NUM_ITEMS << " 个元素" << std::endl;
  std::cout << "耗时: " << duration.count() << " ms" << std::endl;
  std::cout << "吞吐量: " << ops_per_sec / 1'000'000 << " M ops/sec"
            << std::endl;
  std::cout << std::endl;
}

// ============== 示例 3: 结构体传输 ==============

struct Order {
  uint64_t id;
  double price;
  int quantity;
  char side; // 'B' for buy, 'S' for sell
};

void struct_example() {
  std::cout << "=== 结构体传输示例 ===" << std::endl;

  SPSCQueue<Order, 16> order_queue;
  std::atomic<bool> done{false};

  // 交易引擎（Producer）：生成订单
  std::thread trading_engine([&]() {
    for (uint64_t i = 1; i <= 5; ++i) {
      Order order{.id = i,
                  .price = 100.0 + i * 0.5,
                  .quantity = static_cast<int>(i * 100),
                  .side = (i % 2 == 0) ? 'B' : 'S'};

      while (!order_queue.push(order)) {
        std::this_thread::yield();
      }

      std::cout << "[TradingEngine] 发送订单 #" << order.id << std::endl;
    }
    done.store(true, std::memory_order_release);
  });

  // 风控系统（Consumer）：处理订单
  std::thread risk_system([&]() {
    Order order;
    int count = 0;

    while (count < 5) {
      if (order_queue.pop(order)) {
        std::cout << "[RiskSystem] 收到订单 #" << order.id << " | "
                  << (order.side == 'B' ? "买入" : "卖出")
                  << " | 价格: " << order.price << " | 数量: " << order.quantity
                  << std::endl;
        ++count;
      } else {
        std::this_thread::yield();
      }
    }
  });

  trading_engine.join();
  risk_system.join();

  std::cout << std::endl;
}

// ============== 主函数 ==============

int main() {
  basic_example();
  benchmark_example();
  struct_example();

  std::cout << "所有示例运行完毕!" << std::endl;
  return 0;
}