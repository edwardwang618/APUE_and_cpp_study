#include <cassert>
#include <iostream>
#include <mutex>
#include <stdexcept>

template <typename T> class Singleton {
private:
  T value_;
  mutable std::mutex mu_;

  template <typename... Args>
  explicit Singleton(Args &&...args) : value_(std::forward<Args>(args)...) {
    std::cout << "Singleton constructed!" << std::endl;
  }

  ~Singleton() { std::cout << "Singleton destroyed!" << std::endl; }
  Singleton(const Singleton &) = delete;
  Singleton(Singleton &&) = delete;
  Singleton &operator=(const Singleton &) = delete;
  Singleton &operator=(Singleton &&) = delete;

  static Singleton *&getPtr() {
    static Singleton *ptr = nullptr;
    return ptr;
  }

public:
  template <typename... Args> static void init(Args &&...args) {
    static std::once_flag flag;
    std::call_once(flag, [&]() {
      std::cout << "Start creating Singleton!" << std::endl;
      static Singleton instance(std::forward<Args>(args)...);
      // call_once 保证getPtr只会被写一次
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

  void set(const T &value) {
    std::lock_guard<std::mutex> lock(mu_);
    value_ = value;
  }

  void set(T &&value) {
    std::lock_guard<std::mutex> lock(mu_);
    value_ = value;
  }
};

int main() {
  std::cout << "Main start!" << std::endl;
  Singleton<int>::init();
  std::cout << Singleton<int>::getInstance().get() << std::endl;
  Singleton<int>::getInstance().set(10);
  std::cout << Singleton<int>::getInstance().get() << std::endl;

  Singleton<int>::init();
  std::cout << "Main end!" << std::endl;
  return 0;
}