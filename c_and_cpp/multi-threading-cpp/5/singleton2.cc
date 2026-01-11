#include <cassert>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <vector>

template <typename T> class Singleton {
private:
  T value_;
  mutable std::mutex mu_;

  explicit Singleton(T value) : value_(std::move(value)) {
    std::cout << "Singleton constructed!" << std::endl;
  }
  ~Singleton() { std::cout << "Singleton destroyed!" << std::endl; }
  Singleton(const Singleton &) = delete;
  Singleton &operator=(const Singleton &) = delete;

  static Singleton *&getPtr() {
    static Singleton *ptr = nullptr;
    return ptr;
  }

public:
  template <typename... Args> static void init(Args &&...args) {
    static std::once_flag flag;
    std::call_once(flag, [&]() {
      std::cout << "Start creating Singleton!" << std::endl;
      static Singleton instance(T(std::forward<Args>(args)...));
      getPtr() = &instance;
    });
  }

  static Singleton &getInstance() {
    if (!getPtr())
      throw std::runtime_error("Not Initialized!");
    return *getPtr();
  }

  T get() const {
    std::lock_guard<std::mutex> lock(mu_);
    return value_;
  }

  void set(T value) {
    std::lock_guard<std::mutex> lock(mu_);
    value_ = std::move(value);
  }

  template <typename F> auto withValue(F &&func) -> decltype(func(value_)) {
    std::lock_guard<std::mutex> lock(mu_);
    return func(value_);
  }

  template <typename F>
  auto withValue(F &&func) const -> decltype(func(value_)) {
    std::lock_guard<std::mutex> lock(mu_);
    return func(value_);
  }
};

int main() {
  std::cout << "Main start!" << std::endl;
  Singleton<int>::init();
  std::cout << Singleton<int>::getInstance().get() << std::endl;
  Singleton<int>::getInstance().set(10);
  std::cout << Singleton<int>::getInstance().get() << std::endl;

  Singleton<int>::init();

  puts("=======================================");

  Singleton<std::vector<int>>::init(std::vector<int>{1, 2, 3});
  std::cout << Singleton<std::vector<int>>::getInstance().get().size()
            << std::endl;
  Singleton<std::vector<int>>::getInstance().withValue(
      [&](std::vector<int> &v) { v.push_back(4); });
  std::cout << Singleton<std::vector<int>>::getInstance().get().size()
            << std::endl;

  std::cout << "Main end!" << std::endl;
  return 0;
}