#include <chrono>
#include <iostream>
#include <mutex>
#include <stack>
#include <string>
#include <thread>
#include <utility>

template <typename T> class unsafe_stack {
private:
  std::stack<T> stk;
  mutable std::mutex mu;

public:
  unsafe_stack() {}
  unsafe_stack(const unsafe_stack &other) {
    std::lock_guard<std::mutex> lock(other.mu);
    stk = other.stk;
  }

  unsafe_stack &operator=(const unsafe_stack &) = delete;

  template <typename U> void push(U &&val) {
    std::lock_guard<std::mutex> lock(mu);
    stk.push(std::forward<U>(val));
  }

  T pop() {
    std::lock_guard<std::mutex> lock(mu);
    T e = stk.top();
    stk.pop();
    return e;
  }

  bool empty() const {
    std::lock_guard<std::mutex> lock(mu);
    return stk.empty();
  }
};

void test() {
  unsafe_stack<int> stk;
  stk.push(1);
  std::thread t1{[&stk]() {
    if (!stk.empty()) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      stk.pop();
    }
  }};

  std::thread t2{[&stk]() {
    if (!stk.empty()) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      stk.pop();
    }
  }};

  t1.join();
  t2.join();
}

int main() {
  test();
  return 0;
}