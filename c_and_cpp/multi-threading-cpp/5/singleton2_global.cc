#include <iostream>
#include <mutex>

template <typename T> class Singleton {
private:
  T value_;
  mutable std::mutex mu_;

  Singleton() : value_{} { std::cout << "Singleton constructed!" << std::endl; }
  ~Singleton() { std::cout << "Singleton destroyed!" << std::endl; }

  Singleton(const Singleton &) = delete;
  Singleton &operator=(const Singleton &) = delete;

  static Singleton instance; // declaration
public:
  static Singleton &getInstance() { return instance; }

  T get() const {
    std::lock_guard<std::mutex> lock(mu_);
    return value_;
  }

  void set(T value) {
    std::lock_guard<std::mutex> lock(mu_);
    value_ = std::move(value);
  }
};

// Definition - created before main()
template <typename T> Singleton<T> Singleton<T>::instance;

int main() {
  std::cout << "Main start!" << std::endl;
  Singleton<int>::getInstance().set(42);
  std::cout << Singleton<int>::getInstance().get() << std::endl;
  std::cout << "Main end!" << std::endl;
}