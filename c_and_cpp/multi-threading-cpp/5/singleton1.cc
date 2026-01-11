#include <iostream>
#include <mutex>

template <typename T> class Singleton {
private:
  T value_;
  mutable std::mutex mu_;

  explicit Singleton(T value) : value_(std::move(value)) {}
  Singleton(const Singleton &) = delete;
  Singleton &operator=(const Singleton &) = delete;

  static Singleton *&getPtr() {
    static Singleton *ptr = nullptr;
    return ptr;
  }

public:
  template <typename... Args> static void init(Args &&...args) {
    if (getPtr())
      throw std::runtime_error("Already Initialized!");
    static Singleton instance(T(std::forward<Args>(args)...));
    getPtr() = &instance;
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
};

int main() {
  Singleton<int>::init(42);
  std::cout << Singleton<int>::getInstance().get() << std::endl;
  auto &ins = Singleton<int>::getInstance();
  ins.set(1);
  std::cout << ins.get() << std::endl;
}
