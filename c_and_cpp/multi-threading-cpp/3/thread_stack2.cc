#include <chrono>
#include <iostream>
#include <memory>
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

  std::shared_ptr<T> pop() {
    std::lock_guard<std::mutex> lock(mu);
    auto e = std::make_shared<T>(std::move(stk.top()));
    stk.pop();
    return e;
  }

  bool empty() const {
    std::lock_guard<std::mutex> lock(mu);
    return stk.empty();
  }
};

struct BadType {
  static int count;
  BadType() = default;
  BadType(const BadType &) {
    if (count == 0)
      throw std::runtime_error("copy failed");
  }
};

int BadType::count = 0;

void test() {
  unsafe_stack<BadType> stk;
  stk.push(BadType{});

  try {
    auto val = stk.pop(); // Might throw on return copy
                          // Element already popped - LOST!
  } catch (...) {
    // Stack is now empty, but we never got the element
  }
}

int main() {
  test();
  return 0;
}