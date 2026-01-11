#include <iostream>

class Singleton {
public:
  // Get the single instance
  static Singleton &getInstance() {
    static Singleton instance; // Created once, thread-safe in C++11+
    return instance;
  }

  // Delete copy and move
  Singleton(const Singleton &) = delete;
  Singleton &operator=(const Singleton &) = delete;
  Singleton(Singleton &&) = delete;
  Singleton &operator=(Singleton &&) = delete;

  // Your methods
  void doSomething() { std::cout << "Doing something\n"; }
private:
  // Private constructor
  Singleton() { std::cout << "Singleton created\n"; }

  ~Singleton() { std::cout << "Singleton destroyed\n"; }
};

int main() {
  Singleton &s1 = Singleton::getInstance();
  Singleton &s2 = Singleton::getInstance();

  std::cout << (&s1 == &s2) << "\n"; // 1 (same instance)

  s1.doSomething();
}