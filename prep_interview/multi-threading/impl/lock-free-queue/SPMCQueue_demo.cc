#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <thread>
#include <vector>

inline void cpu_pause() {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) ||             \
    defined(_M_IX86)
  __builtin_ia32_pause();
#elif defined(__aarch64__) || defined(_M_ARM64)
  asm volatile("yield" ::: "memory");
#else
  std::this_thread::yield();
#endif
}

template <typename T, size_t Cap> class SPMCQueue {
  alignas(64) std::atomic<size_t> head{0};
  alignas(64) std::atomic<size_t> tail{0};
  alignas(64) std::array<T, Cap + 1> buffer;
  alignas(64) std::array<std::atomic<bool>, Cap + 1> ready;

  static constexpr size_t next_pos(size_t pos) { return (pos + 1) % (Cap + 1); }

public:
  SPMCQueue() {
    for (auto &r : ready)
      r.store(true, std::memory_order_relaxed);
  }

  bool push(const T &val) {
    size_t pos = tail.load(std::memory_order_relaxed);
    size_t next = next_pos(pos);

    if (next == head.load(std::memory_order_relaxed))
      return false;

    while (!ready[pos].load(std::memory_order_relaxed))
      cpu_pause();

    buffer[pos] = val;
    ready[pos].store(false, std::memory_order_relaxed);
    tail.store(next, std::memory_order_release);

    return true;
  }

  bool pop(T &val) {
    size_t pos, next;

    do {
      pos = head.load(std::memory_order_relaxed);

      if (pos == tail.load(std::memory_order_acquire))
        return false;

      next = next_pos(pos);
    } while (!head.compare_exchange_weak(pos, next, std::memory_order_relaxed,
                                         std::memory_order_relaxed));

    val = std::move(buffer[pos]);
    ready[pos].store(true, std::memory_order_release);

    return true;
  }
};

void test_basic() {
  std::cout << "=== Test 1: Basic Functionality ===" << std::endl;

  SPMCQueue<int, 4> q;
  int val;

  assert(q.push(1));
  assert(q.push(2));
  assert(q.push(3));
  assert(q.push(4));
  assert(!q.push(5));

  assert(q.pop(val) && val == 1);
  assert(q.pop(val) && val == 2);
  assert(q.pop(val) && val == 3);
  assert(q.pop(val) && val == 4);
  assert(!q.pop(val));

  std::cout << "PASSED" << std::endl << std::endl;
}

void test_spmc_correctness() {
  std::cout << "=== Test 2: SPMC Correctness ===" << std::endl;

  constexpr int NUM_CONSUMERS = 4;
  constexpr int TOTAL_ITEMS = 40000;

  SPMCQueue<int, 1024> q;
  std::atomic<bool> start{false};
  std::atomic<bool> producer_done{false};

  // 单生产者
  auto producer = [&]() {
    while (!start.load(std::memory_order_acquire))
      ;

    for (int i = 0; i < TOTAL_ITEMS; ++i) {
      while (!q.push(i))
        ;
    }
    producer_done.store(true, std::memory_order_release);
  };

  // 多消费者，每个消费者记录自己收到的值
  std::vector<std::vector<int>> received(NUM_CONSUMERS);
  for (auto &v : received)
    v.reserve(TOTAL_ITEMS / NUM_CONSUMERS + 1000);

  auto consumer = [&](int id) {
    while (!start.load(std::memory_order_acquire))
      ;

    while (true) {
      int val;
      if (q.pop(val)) {
        received[id].push_back(val);
      } else if (producer_done.load(std::memory_order_acquire)) {
        // 队列为空且生产者已完成，再试一次确保没有遗漏
        if (!q.pop(val))
          break;
        received[id].push_back(val);
      }
    }
  };

  std::thread producer_thread(producer);
  std::vector<std::thread> consumers;
  for (int i = 0; i < NUM_CONSUMERS; ++i) {
    consumers.emplace_back(consumer, i);
  }

  start.store(true, std::memory_order_release);

  producer_thread.join();
  for (auto &t : consumers) {
    t.join();
  }

  // 验证：所有值都被消费，且每个值只被消费一次
  size_t total_received = 0;
  for (const auto &v : received)
    total_received += v.size();

  assert(total_received == TOTAL_ITEMS);

  std::vector<bool> seen(TOTAL_ITEMS, false);
  for (const auto &v : received) {
    for (int val : v) {
      assert(val >= 0 && val < TOTAL_ITEMS);
      assert(!seen[val]);
      seen[val] = true;
    }
  }

  std::cout << "PASSED: " << TOTAL_ITEMS << " items distributed to "
            << NUM_CONSUMERS << " consumers" << std::endl;
  for (int i = 0; i < NUM_CONSUMERS; ++i) {
    std::cout << "  Consumer " << i << ": " << received[i].size() << " items"
              << std::endl;
  }
  std::cout << std::endl;
}

void test_stress() {
  std::cout << "=== Test 3: Stress Test ===" << std::endl;

  constexpr int NUM_CONSUMERS = 8;
  constexpr int TOTAL_ITEMS = 800000;

  SPMCQueue<size_t, 256> q;
  std::atomic<bool> start{false};
  std::atomic<bool> producer_done{false};
  std::atomic<size_t> total_consumed{0};

  auto producer = [&]() {
    while (!start.load(std::memory_order_acquire))
      ;

    for (size_t i = 0; i < TOTAL_ITEMS; ++i) {
      while (!q.push(i))
        ;
    }
    producer_done.store(true, std::memory_order_release);
  };

  auto consumer = [&]() {
    size_t count = 0;

    while (true) {
      size_t val;
      if (q.pop(val)) {
        ++count;
      } else if (producer_done.load(std::memory_order_acquire)) {
        // 再试一次确保没有遗漏
        if (!q.pop(val))
          break;
        ++count;
      }
    }

    total_consumed.fetch_add(count, std::memory_order_relaxed);
  };

  auto t1 = std::chrono::high_resolution_clock::now();

  std::thread producer_thread(producer);
  std::vector<std::thread> consumers;
  for (int i = 0; i < NUM_CONSUMERS; ++i) {
    consumers.emplace_back(consumer);
  }

  start.store(true, std::memory_order_release);

  producer_thread.join();
  for (auto &t : consumers) {
    t.join();
  }

  auto t2 = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

  assert(total_consumed.load() == TOTAL_ITEMS);

  double throughput =
      static_cast<double>(TOTAL_ITEMS) / duration.count() * 1000.0;

  std::cout << "PASSED: " << TOTAL_ITEMS << " items in " << duration.count()
            << "ms" << std::endl;
  std::cout << "Throughput: " << static_cast<size_t>(throughput) << " items/sec"
            << std::endl
            << std::endl;
}

void test_edge_cases() {
  std::cout << "=== Test 4: Edge Cases ===" << std::endl;

  // 容量为 1 的队列
  SPMCQueue<int, 1> tiny;
  int val;

  assert(tiny.push(42));
  assert(!tiny.push(43));
  assert(tiny.pop(val) && val == 42);
  assert(!tiny.pop(val));
  assert(tiny.push(100));
  assert(tiny.pop(val) && val == 100);

  // 反复填满清空
  SPMCQueue<int, 8> q;
  for (int round = 0; round < 100; ++round) {
    for (int i = 0; i < 8; ++i) {
      assert(q.push(i));
    }
    assert(!q.push(999));
    for (int i = 0; i < 8; ++i) {
      assert(q.pop(val) && val == i);
    }
    assert(!q.pop(val));
  }

  std::cout << "PASSED" << std::endl << std::endl;
}

int main() {
  test_basic();
  test_spmc_correctness();
  test_stress();
  test_edge_cases();

  std::cout << "=== All Tests Passed ===" << std::endl;
  return 0;
}