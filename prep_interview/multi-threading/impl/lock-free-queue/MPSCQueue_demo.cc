#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <thread>
#include <vector>

// 跨平台 pause 指令
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

template <typename T, size_t Cap> class MPSCQueue {
  alignas(64) std::atomic<size_t> head{0};
  alignas(64) std::atomic<size_t> tail{0};
  alignas(64) std::array<T, Cap + 1> buffer;
  alignas(64) std::array<std::atomic<bool>, Cap + 1> ready;

  static constexpr size_t next_pos(size_t pos) { return (pos + 1) % (Cap + 1); }

public:
  MPSCQueue() {
    for (auto &r : ready)
      r.store(false, std::memory_order_relaxed);
  }

  bool push(const T &val) {
    size_t pos, next;
    do {
      pos = tail.load(std::memory_order_relaxed);
      next = next_pos(pos);
      if (next == head.load(std::memory_order_relaxed))
        return false;
    } while (!tail.compare_exchange_weak(pos, next, std::memory_order_relaxed,
                                         std::memory_order_relaxed));

    buffer[pos] = val;
    ready[pos].store(true, std::memory_order_release);
    return true;
  }

  bool pop(T &val) {
    size_t pos = head.load(std::memory_order_relaxed);

    if (pos == tail.load(std::memory_order_acquire))
      return false;

    while (!ready[pos].load(std::memory_order_acquire))
      cpu_pause();

    val = std::move(buffer[pos]);
    ready[pos].store(false, std::memory_order_relaxed);
    head.store(next_pos(pos), std::memory_order_release);
    return true;
  }
};

// ============================================
// 测试 1: 基本功能测试（单线程）
// ============================================
void test_basic() {
  std::cout << "=== Test 1: Basic Functionality ===" << std::endl;

  MPSCQueue<int, 4> q;
  int val;

  assert(q.push(1));
  assert(q.push(2));
  assert(q.push(3));
  assert(q.push(4));
  assert(!q.push(5)); // 队列已满

  assert(q.pop(val) && val == 1);
  assert(q.pop(val) && val == 2);
  assert(q.pop(val) && val == 3);
  assert(q.pop(val) && val == 4);
  assert(!q.pop(val)); // 队列已空

  std::cout << "PASSED" << std::endl << std::endl;
}

// ============================================
// 测试 2: 多生产者单消费者正确性测试
// ============================================
void test_mpsc_correctness() {
  std::cout << "=== Test 2: MPSC Correctness ===" << std::endl;

  constexpr int NUM_PRODUCERS = 4;
  constexpr int ITEMS_PER_PRODUCER = 10000;
  constexpr int TOTAL_ITEMS = NUM_PRODUCERS * ITEMS_PER_PRODUCER;

  MPSCQueue<int, 1024> q;
  std::atomic<bool> start{false};
  std::atomic<int> produced{0};

  auto producer = [&](int id) {
    while (!start.load(std::memory_order_acquire))
      ;

    int base = id * ITEMS_PER_PRODUCER;
    for (int i = 0; i < ITEMS_PER_PRODUCER; ++i) {
      while (!q.push(base + i))
        ;
      produced.fetch_add(1, std::memory_order_relaxed);
    }
  };

  std::vector<int> received;
  received.reserve(TOTAL_ITEMS);
  std::atomic<bool> done{false};

  auto consumer = [&]() {
    while (!done.load(std::memory_order_acquire) ||
           produced.load(std::memory_order_acquire) >
               static_cast<int>(received.size())) {
      int val;
      if (q.pop(val)) {
        received.push_back(val);
      }
    }
  };

  std::vector<std::thread> producers;
  for (int i = 0; i < NUM_PRODUCERS; ++i) {
    producers.emplace_back(producer, i);
  }
  std::thread consumer_thread(consumer);

  start.store(true, std::memory_order_release);

  for (auto &t : producers) {
    t.join();
  }
  done.store(true, std::memory_order_release);
  consumer_thread.join();

  assert(received.size() == TOTAL_ITEMS);

  std::vector<bool> seen(TOTAL_ITEMS, false);
  for (int v : received) {
    assert(v >= 0 && v < TOTAL_ITEMS);
    assert(!seen[v]);
    seen[v] = true;
  }
  for (bool s : seen) {
    assert(s);
  }

  std::cout << "PASSED: " << TOTAL_ITEMS << " items transferred correctly"
            << std::endl
            << std::endl;
}

// ============================================
// 测试 3: 压力测试（高竞争场景）
// ============================================
void test_stress() {
  std::cout << "=== Test 3: Stress Test ===" << std::endl;

  constexpr int NUM_PRODUCERS = 8;
  constexpr int ITEMS_PER_PRODUCER = 100000;

  MPSCQueue<size_t, 256> q;
  std::atomic<bool> start{false};
  std::atomic<size_t> total_pushed{0};
  std::atomic<size_t> total_popped{0};
  std::atomic<bool> producers_done{false};

  auto producer = [&](int id) {
    while (!start.load(std::memory_order_acquire))
      ;

    for (int i = 0; i < ITEMS_PER_PRODUCER; ++i) {
      while (!q.push(id * ITEMS_PER_PRODUCER + i))
        ;
      total_pushed.fetch_add(1, std::memory_order_relaxed);
    }
  };

  auto consumer = [&]() {
    size_t count = 0;
    while (!producers_done.load(std::memory_order_acquire) ||
           count < NUM_PRODUCERS * ITEMS_PER_PRODUCER) {
      size_t val;
      if (q.pop(val)) {
        ++count;
      }
    }
    total_popped.store(count, std::memory_order_release);
  };

  auto t1 = std::chrono::high_resolution_clock::now();

  std::vector<std::thread> producers;
  for (int i = 0; i < NUM_PRODUCERS; ++i) {
    producers.emplace_back(producer, i);
  }
  std::thread consumer_thread(consumer);

  start.store(true, std::memory_order_release);

  for (auto &t : producers) {
    t.join();
  }
  producers_done.store(true, std::memory_order_release);
  consumer_thread.join();

  auto t2 = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

  assert(total_pushed.load() == NUM_PRODUCERS * ITEMS_PER_PRODUCER);
  assert(total_popped.load() == NUM_PRODUCERS * ITEMS_PER_PRODUCER);

  size_t total = NUM_PRODUCERS * ITEMS_PER_PRODUCER;
  double throughput = static_cast<double>(total) / duration.count() * 1000.0;

  std::cout << "PASSED: " << total << " items in " << duration.count() << "ms"
            << std::endl;
  std::cout << "Throughput: " << static_cast<size_t>(throughput) << " items/sec"
            << std::endl
            << std::endl;
}

// ============================================
// 测试 4: 边界条件测试
// ============================================
void test_edge_cases() {
  std::cout << "=== Test 4: Edge Cases ===" << std::endl;

  MPSCQueue<int, 1> tiny;
  int val;

  assert(tiny.push(42));
  assert(!tiny.push(43));
  assert(tiny.pop(val) && val == 42);
  assert(!tiny.pop(val));
  assert(tiny.push(100));
  assert(tiny.pop(val) && val == 100);

  MPSCQueue<int, 8> q;
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
  test_mpsc_correctness();
  test_stress();
  test_edge_cases();

  std::cout << "=== All Tests Passed ===" << std::endl;
  return 0;
}