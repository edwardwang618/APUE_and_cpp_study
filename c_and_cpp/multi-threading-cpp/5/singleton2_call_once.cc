#include <cassert>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

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
    static std::once_flag flag;
    std::call_once(flag, [&]() {
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
};

// Test 1: Basic functionality
void testBasicUsage() {
  std::cout << "Test 1: Basic usage... ";

  Singleton<int>::init(42);
  assert(Singleton<int>::getInstance().get() == 42);

  Singleton<int>::getInstance().set(100);
  assert(Singleton<int>::getInstance().get() == 100);

  std::cout << "PASSED\n";
}

// Test 2: Same instance returned
void testSameInstance() {
  std::cout << "Test 2: Same instance... ";

  auto &ins1 = Singleton<int>::getInstance();
  auto &ins2 = Singleton<int>::getInstance();

  assert(&ins1 == &ins2);

  std::cout << "PASSED\n";
}

// Test 3: Multiple init calls ignored
void testMultipleInit() {
  std::cout << "Test 3: Multiple init calls... ";

  Singleton<int>::getInstance().set(999);

  Singleton<int>::init(1);
  Singleton<int>::init(2);
  Singleton<int>::init(3);

  // Value should still be 999, not 1, 2, or 3
  assert(Singleton<int>::getInstance().get() == 999);

  std::cout << "PASSED\n";
}

// Test 4: Different types are different singletons
void testDifferentTypes() {
  std::cout << "Test 4: Different types... ";

  Singleton<double>::init(3.14);
  Singleton<std::string>::init("hello");

  assert(Singleton<double>::getInstance().get() == 3.14);
  assert(Singleton<std::string>::getInstance().get() == "hello");

  // Modifying one doesn't affect others
  Singleton<double>::getInstance().set(2.71);
  assert(Singleton<std::string>::getInstance().get() == "hello");

  std::cout << "PASSED\n";
}

// Test 5: Thread safety - concurrent init
void testConcurrentInit() {
  std::cout << "Test 5: Concurrent init... ";

  Singleton<long>::init(0);

  std::vector<std::thread> threads;

  for (int i = 0; i < 100; i++) {
    threads.emplace_back([i]() {
      Singleton<long>::init(i); // all should be ignored
    });
  }

  for (auto &t : threads) {
    t.join();
  }

  // Should still be 0
  assert(Singleton<long>::getInstance().get() == 0);

  std::cout << "PASSED\n";
}

// Test 6: Thread safety - concurrent read/write
void testConcurrentReadWrite() {
  std::cout << "Test 6: Concurrent read/write... ";

  Singleton<int>::getInstance().set(0);

  std::vector<std::thread> writers;
  std::vector<std::thread> readers;

  // Writers increment value
  for (int i = 0; i < 10; i++) {
    writers.emplace_back([]() {
      for (int j = 0; j < 100; j++) {
        auto &ins = Singleton<int>::getInstance();
        int val = ins.get();
        ins.set(val + 1);
      }
    });
  }

  // Readers just read (shouldn't crash)
  for (int i = 0; i < 10; i++) {
    readers.emplace_back([]() {
      for (int j = 0; j < 100; j++) {
        (void)Singleton<int>::getInstance().get();
      }
    });
  }

  for (auto &t : writers)
    t.join();
  for (auto &t : readers)
    t.join();

  // Note: final value may not be 1000 due to race in read-modify-write
  // But no crashes means mutex is working
  std::cout << "PASSED (no crash)\n";
}

// Test 7: Same instance across threads
void testSameInstanceAcrossThreads() {
  std::cout << "Test 7: Same instance across threads... ";

  Singleton<int> *addresses[10];
  std::vector<std::thread> threads;

  for (int i = 0; i < 10; i++) {
    threads.emplace_back(
        [i, &addresses]() { addresses[i] = &Singleton<int>::getInstance(); });
  }

  for (auto &t : threads) {
    t.join();
  }

  // All should have same address
  for (int i = 1; i < 10; i++) {
    assert(addresses[i] == addresses[0]);
  }

  std::cout << "PASSED\n";
}

// Test 8: Uninitialized access throws
void testUninitializedThrows() {
  std::cout << "Test 8: Uninitialized throws... ";

  // Using a new type that hasn't been initialized
  try {
    Singleton<char>::getInstance();
    assert(false); // should not reach here
  } catch (const std::runtime_error &e) {
    assert(std::string(e.what()) == "Not Initialized!");
  }

  std::cout << "PASSED\n";
}

// Test 9: Works with complex types
struct ComplexType {
  int x;
  std::string name;

  ComplexType(int x, std::string name) : x(x), name(std::move(name)) {}
};

void testComplexType() {
  std::cout << "Test 9: Complex type... ";

  Singleton<ComplexType>::init(42, "test");

  assert(Singleton<ComplexType>::getInstance().get().x == 42);
  assert(Singleton<ComplexType>::getInstance().get().name == "test");

  Singleton<ComplexType>::getInstance().set(ComplexType{100, "updated"});

  assert(Singleton<ComplexType>::getInstance().get().x == 100);
  assert(Singleton<ComplexType>::getInstance().get().name == "updated");

  std::cout << "PASSED\n";
}

int main() {
  std::cout << "=== Singleton Tests ===\n\n";

  testBasicUsage();
  testSameInstance();
  testMultipleInit();
  testDifferentTypes();
  testConcurrentInit();
  testConcurrentReadWrite();
  testSameInstanceAcrossThreads();
  testUninitializedThrows();
  testComplexType();

  std::cout << "\n=== All tests passed! ===\n";

  return 0;
}