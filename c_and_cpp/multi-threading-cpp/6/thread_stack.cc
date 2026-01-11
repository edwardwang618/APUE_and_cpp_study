#include <cassert>
#include <condition_variable>
#include <cstddef>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <stack>
#include <string>
#include <thread>
#include <vector>

template <typename T> class safe_stack {
private:
  mutable std::mutex mu_;
  std::stack<T> stk_;
  std::condition_variable cv_;

public:
  safe_stack() = default;
  safe_stack(const safe_stack &other) {
    std::lock_guard<std::mutex> lk(other.mu_);
    stk_ = other.stk_;
  }

  safe_stack &operator=(const safe_stack &other) {
    if (this == &other)
      return *this;
    std::scoped_lock lk(mu_, other.mu_);
    stk_ = other.stk_;
    return *this;
  }

  safe_stack(safe_stack &&other) {
    std::lock_guard<std::mutex> lk(other.mu_);
    stk_ = std::move(other.stk_);
  }

  safe_stack &operator=(safe_stack &&other) {
    if (this == &other)
      return *this;
    std::scoped_lock lk(mu_, other.mu_);
    stk_ = std::move(other.stk_);
    return *this;
  }

  void push(T value) {
    std::lock_guard<std::mutex> lk(mu_);
    stk_.push(std::move(value));
    cv_.notify_one();
  }

  std::shared_ptr<T> pop() {
    std::unique_lock<std::mutex> lk(mu_);
    cv_.wait(lk, [this]() { return !stk_.empty(); });
    auto res = std::make_shared<T>(std::move(stk_.top()));
    stk_.pop();
    return res;
  }

  std::shared_ptr<T> try_pop() {
    std::lock_guard<std::mutex> lk(mu_);
    if (stk_.empty())
      return nullptr;
    auto res = std::make_shared<T>(std::move(stk_.top()));
    stk_.pop();
    return res;
  }

  std::optional<T> top() const {
    std::unique_lock<std::mutex> lk(mu_);
    if (!stk_.empty())
      return stk_.top();
    return std::nullopt;
  }

  size_t size() const {
    std::lock_guard<std::mutex> lk(mu_);
    return stk_.size();
  }

  bool empty() const {
    std::lock_guard<std::mutex> lk(mu_);
    return stk_.empty();
  }
};

void test_basic_push_pop() {
  safe_stack<int> s;
  s.push(1);
  s.push(2);
  s.push(3);

  assert(*s.pop() == 3);
  assert(*s.pop() == 2);
  assert(*s.pop() == 1);
  std::cout << "test_basic_push_pop passed\n";
}

void test_try_pop_empty() {
  safe_stack<int> s;
  assert(s.try_pop() == nullptr);
  s.push(42);
  assert(*s.try_pop() == 42);
  assert(s.try_pop() == nullptr);
  std::cout << "test_try_pop_empty passed\n";
}

void test_top() {
  safe_stack<int> s;
  assert(!s.top().has_value());
  s.push(10);
  assert(s.top().value() == 10);
  assert(s.top().value() == 10); // still there
  std::cout << "test_top passed\n";
}

void test_size_empty() {
  safe_stack<int> s;
  assert(s.empty());
  assert(s.size() == 0);
  s.push(1);
  assert(!s.empty());
  assert(s.size() == 1);
  std::cout << "test_size_empty passed\n";
}

void test_copy() {
  safe_stack<int> s1;
  s1.push(1);
  s1.push(2);

  safe_stack<int> s2(s1);
  assert(*s2.pop() == 2);
  assert(*s2.pop() == 1);

  // original unchanged
  assert(*s1.pop() == 2);
  std::cout << "test_copy passed\n";
}

void test_move() {
  safe_stack<int> s1;
  s1.push(1);
  s1.push(2);

  safe_stack<int> s2(std::move(s1));
  assert(*s2.pop() == 2);
  assert(*s2.pop() == 1);
  assert(s1.empty()); // moved from
  std::cout << "test_move passed\n";
}

void test_concurrent_push_pop() {
  safe_stack<int> s;
  const int num_items = 1000;

  std::thread producer([&]() {
    for (int i = 0; i < num_items; i++) {
      s.push(i);
    }
  });

  std::thread consumer([&]() {
    for (int i = 0; i < num_items; i++) {
      s.pop(); // blocks until available
    }
  });

  producer.join();
  consumer.join();
  assert(s.empty());
  std::cout << "test_concurrent_push_pop passed\n";
}

void test_multiple_producers_consumers() {
  safe_stack<int> s;
  const int items_per_thread = 500;
  const int num_producers = 4;
  const int num_consumers = 4;
  std::atomic<int> consumed{0};

  std::vector<std::thread> producers;
  std::vector<std::thread> consumers;

  for (int i = 0; i < num_producers; i++) {
    producers.emplace_back([&]() {
      for (int j = 0; j < items_per_thread; j++) {
        s.push(j);
      }
    });
  }

  for (int i = 0; i < num_consumers; i++) {
    consumers.emplace_back([&]() {
      for (int j = 0; j < items_per_thread; j++) {
        s.pop();
        consumed++;
      }
    });
  }

  for (auto &t : producers)
    t.join();
  for (auto &t : consumers)
    t.join();

  assert(consumed == num_producers * items_per_thread);
  assert(s.empty());
  std::cout << "test_multiple_producers_consumers passed\n";
}

void test_blocking_pop() {
  safe_stack<int> s;
  std::atomic<bool> popped{false};

  std::thread consumer([&]() {
    s.pop(); // should block
    popped = true;
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  assert(!popped); // still waiting

  s.push(42);
  consumer.join();
  assert(popped);
  std::cout << "test_blocking_pop passed\n";
}

int main() {
  test_basic_push_pop();
  test_try_pop_empty();
  test_top();
  test_size_empty();
  test_copy();
  test_move();
  test_concurrent_push_pop();
  test_multiple_producers_consumers();
  test_blocking_pop();

  std::cout << "\nAll tests passed!\n";
}