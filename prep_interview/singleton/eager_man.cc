#include <iostream>
#include <mutex>

template <typename T> class Singleton {
private:
  T value_;
  mutable std::mutex mu_;

  Singleton() : value_{} { std::cout << "Singleton constructed!" << std::endl; }
  ~Singleton() { std::cout << "Singleton destroyed!" << std::endl; }

  Singleton(const Singleton &) = delete;
  Singleton(Singleton &&) = delete;
  Singleton &operator=(const Singleton &) = delete;
  Singleton &operator=(Singleton &&) = delete;

  // 类的静态成员变量要在类外面定义，但是也可以加上inline让它在类的内部定义。这里没有加inline
  static inline Singleton instance; // declaration
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

int main() {
  std::cout << "Main start!" << std::endl;
  Singleton<int>::getInstance().set(42);
  std::cout << Singleton<int>::getInstance().get() << std::endl;

  auto &s1 = Singleton<int>::getInstance();
  auto &s2 = Singleton<int>::getInstance();
  std::cout << (&s1 == &s2) << std::endl;
  std::cout << "Main end!" << std::endl;
}
