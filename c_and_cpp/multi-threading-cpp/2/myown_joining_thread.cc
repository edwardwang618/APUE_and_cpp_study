#include <chrono>
#include <iostream>
#include <thread>
#include <utility>

class joining_thread {
private:
  std::thread t;

public:
  // Default constructor
  joining_thread() noexcept = default;

  // Templated constructor (same as std::thread)
  template <typename Func, typename... Args>
  explicit joining_thread(Func &&f, Args &&...args)
      : t(std::forward<Func>(f), std::forward<Args>(args)...) {}

  // Construct from std::thread
  explicit joining_thread(std::thread other) noexcept : t(std::move(other)) {}

  // Destructor - auto joins!
  ~joining_thread() {
    if (t.joinable()) {
      t.join();
    }
  }

  // Move constructor
  joining_thread(joining_thread &&other) noexcept : t(std::move(other.t)) {}

  // Move assignment - joins before reassigning!
  joining_thread &operator=(joining_thread &&other) noexcept {
    if (this != &other) {
      if (t.joinable()) {
        t.join();
      }
      t = std::move(other.t);
    }
    return *this;
  }

  // Move assign from std::thread
  joining_thread &operator=(std::thread other) noexcept {
    if (t.joinable()) {
      t.join();
    }
    t = std::move(other);
    return *this;
  }

  // No copying
  joining_thread(const joining_thread &) = delete;
  joining_thread &operator=(const joining_thread &) = delete;

  // Forward std::thread methods
  void join() { t.join(); }
  void detach() { t.detach(); }
  bool joinable() const noexcept { return t.joinable(); }
  std::thread::id get_id() const noexcept { return t.get_id(); }
  std::thread::native_handle_type native_handle() { return t.native_handle(); }

  // Swap
  void swap(joining_thread &other) noexcept { t.swap(other.t); }

  // Release ownership (convert back to std::thread)
  std::thread release() noexcept { return std::move(t); }
};

// Non-member swap
void swap(joining_thread &a, joining_thread &b) noexcept { a.swap(b); }

// ============ Test ============

void some_function() {
  std::cout << "some_function running" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "some_function done" << std::endl;
}

void some_other_function() {
  std::cout << "some_other_function running" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "some_other_function done" << std::endl;
}

void test_basic() {
  std::cout << "=== Test: Basic (auto join) ===" << std::endl;
  joining_thread t(some_function);
  std::cout << "Main continues..." << std::endl;
  // No need to join!
}

void test_reassignment() {
  std::cout << "\n=== Test: Reassignment (was dangerous) ===" << std::endl;

  joining_thread t1(some_function);
  joining_thread t2 = std::move(t1);

  t1 = joining_thread(some_other_function);

  joining_thread t3;
  t3 = std::move(t2);

  // This was dangerous with std::thread, safe now!
  t1 = std::move(t3);

  std::cout << "No crash!" << std::endl;
}

void test_with_args() {
  std::cout << "\n=== Test: With arguments ===" << std::endl;

  joining_thread t(
      [](int x, const std::string &msg) {
        std::cout << msg << ": " << x << std::endl;
      },
      42, "The answer is");
}

int main() {
  test_basic();
  test_reassignment();
  test_with_args();

  std::thread t([](const char *s) { printf("%s\n", s); }, "Hello");

  joining_thread jt(std::move(t));

  std::cout << "\nAll done!" << std::endl;
  return 0;
}