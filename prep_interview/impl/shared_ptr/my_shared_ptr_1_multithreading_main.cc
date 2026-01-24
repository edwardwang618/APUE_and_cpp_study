#include "my_shared_ptr_1_multithreading.h"
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

// 你的 SharedPtr 代码放这里

struct Foo {
  std::atomic<int> value{0};
  static std::atomic<int> count;
  Foo() { ++count; }
  ~Foo() { --count; }
};
std::atomic<int> Foo::count{0};

int main() {
  // 测试1：多线程拷贝和析构
  {
    SharedPtr<Foo> p(new Foo());
    std::vector<std::thread> threads;

    for (int i = 0; i < 100; ++i) {
      threads.emplace_back([&p]() {
        for (int j = 0; j < 10000; ++j) {
          SharedPtr<Foo> local = p; // 拷贝
          local->value.fetch_add(1, std::memory_order_relaxed);
        } // 析构
      });
    }

    for (auto &t : threads) {
      t.join();
    }

    assert(p.use_count() == 1);
    assert(p->value == 100 * 10000);
    assert(Foo::count == 1);
  }
  assert(Foo::count == 0);
  std::cout << "Test 1 passed: concurrent copy/destruct\n";

  // 测试2：多线程竞争最后一个引用
  for (int round = 0; round < 100; ++round) {
    SharedPtr<Foo> p(new Foo());
    std::vector<std::thread> threads;
    std::vector<SharedPtr<Foo>> copies(100, p);
    p.reset(); // 释放原始的

    for (int i = 0; i < 100; ++i) {
      threads.emplace_back([&copies, i]() {
        copies[i].reset(); // 每个线程释放自己的拷贝
      });
    }

    for (auto &t : threads) {
      t.join();
    }
  }
  assert(Foo::count == 0);
  std::cout << "Test 2 passed: race to last reference\n";

  std::cout << "\nAll multithreaded tests passed!\n";
  return 0;
}