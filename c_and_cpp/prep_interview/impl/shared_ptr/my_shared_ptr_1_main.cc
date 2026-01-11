#include "my_shared_ptr_1.h"
#include <cassert>
#include <iostream>
// 你的 SharedPtr 代码放这里

struct Foo {
  int value;
  static int count;
  Foo(int v = 0) : value(v) { ++count; }
  ~Foo() { --count; }
};
int Foo::count = 0;

int main() {
  // 测试1: 基本构造和析构
  {
    SharedPtr<Foo> p(new Foo(42));
    assert(p.use_count() == 1);
    assert(p->value == 42);
    assert((*p).value == 42);
    assert(Foo::count == 1);
  }
  assert(Foo::count == 0);
  std::cout << "Test 1 passed: basic construct/destruct\n";

  // 测试2: 拷贝构造
  {
    SharedPtr<Foo> p1(new Foo(10));
    SharedPtr<Foo> p2(p1);
    assert(p1.use_count() == 2);
    assert(p2.use_count() == 2);
    assert(p1.get() == p2.get());
    assert(Foo::count == 1);
  }
  assert(Foo::count == 0);
  std::cout << "Test 2 passed: copy construct\n";

  // 测试3: 拷贝赋值
  {
    SharedPtr<Foo> p1(new Foo(1));
    SharedPtr<Foo> p2(new Foo(2));
    assert(Foo::count == 2);
    p2 = p1;
    assert(Foo::count == 1);
    assert(p1.use_count() == 2);
    assert(p2->value == 1);
  }
  assert(Foo::count == 0);
  std::cout << "Test 3 passed: copy assign\n";

  // 测试4: 移动构造
  {
    SharedPtr<Foo> p1(new Foo(100));
    SharedPtr<Foo> p2(std::move(p1));
    assert(!p1);
    assert(p1.use_count() == 0);
    assert(p2.use_count() == 1);
    assert(p2->value == 100);
  }
  assert(Foo::count == 0);
  std::cout << "Test 4 passed: move construct\n";

  // 测试5: 移动赋值
  {
    SharedPtr<Foo> p1(new Foo(200));
    SharedPtr<Foo> p2;
    p2 = std::move(p1);
    assert(!p1);
    assert(p2.use_count() == 1);
    assert(p2->value == 200);
  }
  assert(Foo::count == 0);
  std::cout << "Test 5 passed: move assign\n";

  // 测试6: reset
  {
    SharedPtr<Foo> p(new Foo(1));
    p.reset(new Foo(2));
    assert(Foo::count == 1);
    assert(p->value == 2);
    p.reset();
    assert(!p);
    assert(Foo::count == 0);
  }
  std::cout << "Test 6 passed: reset\n";

  // 测试7: swap
  {
    SharedPtr<Foo> p1(new Foo(111));
    SharedPtr<Foo> p2(new Foo(222));
    p1.swap(p2);
    assert(p1->value == 222);
    assert(p2->value == 111);
  }
  assert(Foo::count == 0);
  std::cout << "Test 7 passed: swap\n";

  // 测试8: nullptr
  {
    SharedPtr<Foo> p1(nullptr);
    SharedPtr<Foo> p2 = nullptr;
    assert(!p1);
    assert(!p2);
    assert(p1.use_count() == 0);
  }
  std::cout << "Test 8 passed: nullptr\n";

  // 测试9: make_shared
  {
    auto p = make_shared<Foo>(999);
    assert(p.use_count() == 1);
    assert(p->value == 999);
  }
  assert(Foo::count == 0);
  std::cout << "Test 9 passed: make_shared\n";

  // 测试10: 自赋值
  {
    SharedPtr<Foo> p(new Foo(50));
    p = p;
    assert(p.use_count() == 1);
    assert(p->value == 50);
  }
  assert(Foo::count == 0);
  std::cout << "Test 10 passed: self assign\n";

  std::cout << "\nAll tests passed!\n";
  return 0;
}