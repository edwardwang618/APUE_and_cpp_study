#include "my_unique_ptr.h"
#include <cassert>
#include <iostream>
#include <string>
#include <utility>

// ============================================
// 测试辅助
// ============================================

struct Counter {
  static int constructed;
  static int destroyed;
  static void reset() {
    constructed = 0;
    destroyed = 0;
  }
};

int Counter::constructed = 0;
int Counter::destroyed = 0;

struct Foo : Counter {
  int value;

  Foo(int v = 0) : value(v) {
    ++constructed;
    std::cout << "  Foo(" << value << ") constructed\n";
  }

  ~Foo() {
    ++destroyed;
    std::cout << "  Foo(" << value << ") destroyed\n";
  }

  int getValue() const { return value; }
};

// 用于测试数组版本阻止派生类转换
struct Base : Counter {
  int base_value;

  Base(int v = 0) : base_value(v) { ++constructed; }

  virtual ~Base() { ++destroyed; }
};

struct Derived : Base {
  int derived_value;
  char padding[100]; // 故意让 sizeof 不同

  Derived(int v = 0) : Base(v), derived_value(v * 2) {}
};

// ============================================
// 单对象测试用例
// ============================================

void test_default_constructor() {
  std::cout << "[TEST] Default constructor\n";

  UniquePtr<int> p;
  assert(p.get() == nullptr);
  assert(!p);

  std::cout << "[PASS]\n\n";
}

void test_pointer_constructor() {
  std::cout << "[TEST] Pointer constructor\n";
  Counter::reset();

  {
    UniquePtr<Foo> p(new Foo(42));
    assert(p.get() != nullptr);
    assert(p);
    assert((*p).value == 42);
    assert(p->getValue() == 42);
    assert(Counter::constructed == 1);
    assert(Counter::destroyed == 0);
  }

  assert(Counter::destroyed == 1);
  std::cout << "[PASS]\n\n";
}

void test_move_constructor() {
  std::cout << "[TEST] Move constructor\n";
  Counter::reset();

  {
    UniquePtr<Foo> p1(new Foo(100));
    Foo *raw = p1.get();

    UniquePtr<Foo> p2(std::move(p1));

    assert(p1.get() == nullptr);
    assert(!p1);
    assert(p2.get() == raw);
    assert(p2->value == 100);
    assert(Counter::destroyed == 0);
  }

  assert(Counter::destroyed == 1);
  std::cout << "[PASS]\n\n";
}

void test_move_assignment() {
  std::cout << "[TEST] Move assignment\n";
  Counter::reset();

  {
    UniquePtr<Foo> p1(new Foo(1));
    UniquePtr<Foo> p2(new Foo(2));
    Foo *raw1 = p1.get();

    assert(Counter::constructed == 2);
    assert(Counter::destroyed == 0);

    p2 = std::move(p1);

    assert(p1.get() == nullptr);
    assert(p2.get() == raw1);
    assert(p2->value == 1);
    assert(Counter::destroyed == 1); // Foo(2) 被销毁
  }

  assert(Counter::destroyed == 2);
  std::cout << "[PASS]\n\n";
}

void test_self_move_assignment() {
  std::cout << "[TEST] Self move assignment\n";
  Counter::reset();

  {
    UniquePtr<Foo> p(new Foo(42));
    Foo *raw = p.get();

    p = std::move(p);

    // 自移动后，行为应该合理（不崩溃，不泄漏）
    // 标准库的行为：指针保持不变或变为 nullptr
    // 至少不应该 double-free
    assert(Counter::destroyed <= 1);
  }

  std::cout << "[PASS]\n\n";
}

void test_release() {
  std::cout << "[TEST] release()\n";
  Counter::reset();

  Foo *raw = nullptr;
  {
    UniquePtr<Foo> p(new Foo(42));
    raw = p.release();

    assert(p.get() == nullptr);
    assert(!p);
    assert(raw != nullptr);
    assert(raw->value == 42);
    assert(Counter::destroyed == 0);
  }

  assert(Counter::destroyed == 0); // 不应该被销毁
  delete raw;                      // 手动删除
  assert(Counter::destroyed == 1);

  std::cout << "[PASS]\n\n";
}

void test_reset() {
  std::cout << "[TEST] reset()\n";
  Counter::reset();

  {
    UniquePtr<Foo> p(new Foo(1));
    assert(Counter::constructed == 1);

    // reset 到新指针
    p.reset(new Foo(2));
    assert(Counter::constructed == 2);
    assert(Counter::destroyed == 1); // Foo(1) 被销毁
    assert(p->value == 2);

    // reset 到 nullptr
    p.reset();
    assert(Counter::destroyed == 2); // Foo(2) 被销毁
    assert(p.get() == nullptr);
  }

  assert(Counter::destroyed == 2);
  std::cout << "[PASS]\n\n";
}

void test_reset_same_pointer() {
  std::cout << "[TEST] reset() with same pointer\n";
  Counter::reset();

  {
    Foo *raw = new Foo(42);
    UniquePtr<Foo> p(raw);

    // reset 到相同指针 - 不应该 double-free
    // 标准行为是未定义的，但好的实现应该处理
    // p.reset(raw);  // 危险！通常不测试这个

    p.reset(nullptr);
    assert(Counter::destroyed == 1);
  }

  std::cout << "[PASS]\n\n";
}

void test_swap() {
  std::cout << "[TEST] swap()\n";
  Counter::reset();

  {
    UniquePtr<Foo> p1(new Foo(1));
    UniquePtr<Foo> p2(new Foo(2));
    Foo *raw1 = p1.get();
    Foo *raw2 = p2.get();

    p1.swap(p2);

    assert(p1.get() == raw2);
    assert(p2.get() == raw1);
    assert(p1->value == 2);
    assert(p2->value == 1);
    assert(Counter::destroyed == 0);
  }

  assert(Counter::destroyed == 2);
  std::cout << "[PASS]\n\n";
}

void test_swap_with_nullptr() {
  std::cout << "[TEST] swap() with nullptr\n";
  Counter::reset();

  {
    UniquePtr<Foo> p1(new Foo(42));
    UniquePtr<Foo> p2;
    Foo *raw = p1.get();

    p1.swap(p2);

    assert(p1.get() == nullptr);
    assert(p2.get() == raw);
  }

  std::cout << "[PASS]\n\n";
}

void test_dereference() {
  std::cout << "[TEST] operator* and operator->\n";

  UniquePtr<Foo> p(new Foo(42));

  // operator*
  Foo &ref = *p;
  assert(ref.value == 42);
  ref.value = 100;
  assert(p->value == 100);

  // operator->
  assert(p->getValue() == 100);

  std::cout << "[PASS]\n\n";
}

void test_bool_conversion() {
  std::cout << "[TEST] operator bool\n";

  UniquePtr<int> p1;
  UniquePtr<int> p2(new int(42));

  assert(!p1);
  assert(p2);

  if (p1) {
    assert(false); // 不应该到这里
  }

  if (p2) {
    // 应该到这里
  } else {
    assert(false);
  }

  std::cout << "[PASS]\n\n";
}

void test_make_unique() {
  std::cout << "[TEST] make_unique\n";
  Counter::reset();

  {
    auto p = make_unique<Foo>(42);
    assert(p.get() != nullptr);
    assert(p->value == 42);
    assert(Counter::constructed == 1);
  }

  assert(Counter::destroyed == 1);
  std::cout << "[PASS]\n\n";
}

void test_make_unique_default() {
  std::cout << "[TEST] make_unique with default constructor\n";
  Counter::reset();

  {
    auto p = make_unique<Foo>();
    assert(p->value == 0);
  }

  assert(Counter::destroyed == 1);
  std::cout << "[PASS]\n\n";
}

void test_with_string() {
  std::cout << "[TEST] UniquePtr<std::string>\n";

  {
    UniquePtr<std::string> p(new std::string("hello"));
    assert(*p == "hello");
    assert(p->size() == 5);

    *p = "world";
    assert(*p == "world");
  }

  std::cout << "[PASS]\n\n";
}

void test_nullptr_assignment() {
  std::cout << "[TEST] nullptr assignment\n";
  Counter::reset();

  {
    UniquePtr<Foo> p(new Foo(42));
    p = nullptr; // 如果你实现了 operator=(nullptr_t)

    // 或者用 reset
    // p.reset();

    // 二选一测试，取决于你是否实现了 nullptr 赋值
  }

  std::cout << "[PASS]\n\n";
}

// ============================================
// 数组版本测试用例
// ============================================

void test_array_default_constructor() {
  std::cout << "[TEST][Array] Default constructor\n";

  UniquePtr<int[]> p;
  assert(p.get() == nullptr);
  assert(!p);

  std::cout << "[PASS]\n\n";
}

void test_array_pointer_constructor() {
  std::cout << "[TEST][Array] Pointer constructor\n";
  Counter::reset();

  {
    UniquePtr<Foo[]> p(new Foo[3]{{1}, {2}, {3}});
    assert(p.get() != nullptr);
    assert(p);
    assert(Counter::constructed == 3);
    assert(Counter::destroyed == 0);
  }

  assert(Counter::destroyed == 3);
  std::cout << "[PASS]\n\n";
}

void test_array_subscript() {
  std::cout << "[TEST][Array] operator[]\n";
  Counter::reset();

  {
    UniquePtr<Foo[]> p(new Foo[3]{{10}, {20}, {30}});

    // 读取
    assert(p[0].value == 10);
    assert(p[1].value == 20);
    assert(p[2].value == 30);

    // 修改
    p[1].value = 200;
    assert(p[1].value == 200);
  }

  std::cout << "[PASS]\n\n";
}

void test_array_move_constructor() {
  std::cout << "[TEST][Array] Move constructor\n";
  Counter::reset();

  {
    UniquePtr<Foo[]> p1(new Foo[2]{{1}, {2}});
    Foo *raw = p1.get();

    UniquePtr<Foo[]> p2(std::move(p1));

    assert(p1.get() == nullptr);
    assert(!p1);
    assert(p2.get() == raw);
    assert(p2[0].value == 1);
    assert(p2[1].value == 2);
    assert(Counter::destroyed == 0);
  }

  assert(Counter::destroyed == 2);
  std::cout << "[PASS]\n\n";
}

void test_array_move_assignment() {
  std::cout << "[TEST][Array] Move assignment\n";
  Counter::reset();

  {
    UniquePtr<Foo[]> p1(new Foo[2]{{1}, {2}});
    UniquePtr<Foo[]> p2(new Foo[2]{{3}, {4}});
    Foo *raw1 = p1.get();

    assert(Counter::constructed == 4);
    assert(Counter::destroyed == 0);

    p2 = std::move(p1);

    assert(p1.get() == nullptr);
    assert(p2.get() == raw1);
    assert(p2[0].value == 1);
    assert(Counter::destroyed == 2); // p2 原来的数组被销毁
  }

  assert(Counter::destroyed == 4);
  std::cout << "[PASS]\n\n";
}

void test_array_self_move_assignment() {
  std::cout << "[TEST][Array] Self move assignment\n";
  Counter::reset();

  {
    UniquePtr<Foo[]> p(new Foo[2]{{1}, {2}});

    p = std::move(p);

    // 自移动后不应该崩溃或 double-free
    assert(Counter::destroyed <= 2);
  }

  std::cout << "[PASS]\n\n";
}

void test_array_release() {
  std::cout << "[TEST][Array] release()\n";
  Counter::reset();

  Foo *raw = nullptr;
  {
    UniquePtr<Foo[]> p(new Foo[2]{{1}, {2}});
    raw = p.release();

    assert(p.get() == nullptr);
    assert(!p);
    assert(raw != nullptr);
    assert(Counter::destroyed == 0);
  }

  assert(Counter::destroyed == 0);
  delete[] raw;
  assert(Counter::destroyed == 2);

  std::cout << "[PASS]\n\n";
}

void test_array_reset() {
  std::cout << "[TEST][Array] reset()\n";
  Counter::reset();

  {
    UniquePtr<Foo[]> p(new Foo[2]{{1}, {2}});
    assert(Counter::constructed == 2);

    // reset 到新数组
    p.reset(new Foo[3]{{10}, {20}, {30}});
    assert(Counter::constructed == 5);
    assert(Counter::destroyed == 2);
    assert(p[2].value == 30);

    // reset 到 nullptr
    p.reset();
    assert(Counter::destroyed == 5);
    assert(p.get() == nullptr);
  }

  std::cout << "[PASS]\n\n";
}

void test_array_swap() {
  std::cout << "[TEST][Array] swap()\n";
  Counter::reset();

  {
    UniquePtr<Foo[]> p1(new Foo[2]{{1}, {2}});
    UniquePtr<Foo[]> p2(new Foo[2]{{3}, {4}});
    Foo *raw1 = p1.get();
    Foo *raw2 = p2.get();

    p1.swap(p2);

    assert(p1.get() == raw2);
    assert(p2.get() == raw1);
    assert(p1[0].value == 3);
    assert(p2[0].value == 1);
    assert(Counter::destroyed == 0);
  }

  assert(Counter::destroyed == 4);
  std::cout << "[PASS]\n\n";
}

void test_array_swap_with_nullptr() {
  std::cout << "[TEST][Array] swap() with nullptr\n";
  Counter::reset();

  {
    UniquePtr<Foo[]> p1(new Foo[2]{{1}, {2}});
    UniquePtr<Foo[]> p2;
    Foo *raw = p1.get();

    p1.swap(p2);

    assert(p1.get() == nullptr);
    assert(p2.get() == raw);
  }

  std::cout << "[PASS]\n\n";
}

void test_array_bool_conversion() {
  std::cout << "[TEST][Array] operator bool\n";

  UniquePtr<int[]> p1;
  UniquePtr<int[]> p2(new int[3]{1, 2, 3});

  assert(!p1);
  assert(p2);

  if (p1) {
    assert(false);
  }

  if (p2) {
    // 应该到这里
  } else {
    assert(false);
  }

  std::cout << "[PASS]\n\n";
}

void test_array_nullptr_constructor() {
  std::cout << "[TEST][Array] nullptr constructor\n";

  UniquePtr<int[]> p(nullptr);
  assert(p.get() == nullptr);
  assert(!p);

  std::cout << "[PASS]\n\n";
}

void test_array_nullptr_assignment() {
  std::cout << "[TEST][Array] nullptr assignment\n";
  Counter::reset();

  {
    UniquePtr<Foo[]> p(new Foo[2]{{1}, {2}});
    assert(Counter::constructed == 2);

    p = nullptr;

    assert(p.get() == nullptr);
    assert(Counter::destroyed == 2);
  }

  std::cout << "[PASS]\n\n";
}

void test_array_primitive_types() {
  std::cout << "[TEST][Array] Primitive types (int)\n";

  {
    UniquePtr<int[]> p(new int[5]{10, 20, 30, 40, 50});

    assert(p[0] == 10);
    assert(p[4] == 50);

    p[2] = 300;
    assert(p[2] == 300);
  }

  std::cout << "[PASS]\n\n";
}

void test_array_const_access() {
  std::cout << "[TEST][Array] const access\n";

  {
    const UniquePtr<int[]> p(new int[3]{1, 2, 3});

    // const 对象也应该能访问
    assert(p.get() != nullptr);
    assert(p);
    assert(p[0] == 1);
    assert(p[1] == 2);
  }

  std::cout << "[PASS]\n\n";
}

void test_make_unique_array() {
  std::cout << "[TEST][Array] make_unique<T[]>(size)\n";
  Counter::reset();

  {
    auto p = make_unique<Foo[]>(3);
    assert(p.get() != nullptr);
    assert(Counter::constructed == 3);

    // 值初始化，value 应该是 0
    assert(p[0].value == 0);
    assert(p[1].value == 0);
    assert(p[2].value == 0);
  }

  assert(Counter::destroyed == 3);
  std::cout << "[PASS]\n\n";
}

void test_make_unique_array_primitive() {
  std::cout << "[TEST][Array] make_unique<int[]>(size)\n";

  {
    auto p = make_unique<int[]>(5);
    assert(p.get() != nullptr);

    // 值初始化，应该是 0
    for (int i = 0; i < 5; ++i) {
      assert(p[i] == 0);
    }

    // 修改
    p[0] = 100;
    p[4] = 400;
    assert(p[0] == 100);
    assert(p[4] == 400);
  }

  std::cout << "[PASS]\n\n";
}

void test_array_large_array() {
  std::cout << "[TEST][Array] Large array\n";

  {
    const size_t size = 1000;
    auto p = make_unique<int[]>(size);

    for (size_t i = 0; i < size; ++i) {
      p[i] = static_cast<int>(i * 2);
    }

    for (size_t i = 0; i < size; ++i) {
      assert(p[i] == static_cast<int>(i * 2));
    }
  }

  std::cout << "[PASS]\n\n";
}

// ============================================
// 编译期测试 - 确保数组版本没有 operator* 和 operator->
// ============================================

// 取消注释以下代码应该导致编译错误：
/*
void test_array_no_dereference_should_fail() {
  UniquePtr<int[]> p(new int[3]);
  *p;      // 应该编译错误：数组版本没有 operator*
  p->foo;  // 应该编译错误：数组版本没有 operator->
}
*/

// ============================================
// 编译期测试 - 确保单对象版本没有 operator[]
// ============================================

// 取消注释以下代码应该导致编译错误：
/*
void test_single_no_subscript_should_fail() {
  UniquePtr<int> p(new int(42));
  p[0];  // 应该编译错误：单对象版本没有 operator[]
}
*/

// ============================================
// 编译期测试 - 阻止派生类数组到基类数组的转换
// ============================================

// 取消注释以下代码应该导致编译错误：
/*
void test_array_no_derived_conversion_should_fail() {
  // 这是危险的！delete[] 会出问题
  UniquePtr<Base[]> p(new Derived[10]);  // 应该编译错误
}
*/

// ============================================
// 编译期测试 - 确保拷贝被禁止
// ============================================

// 取消注释以下代码应该导致编译错误：
/*
void test_array_copy_should_fail() {
  UniquePtr<int[]> p1(new int[3]);
  UniquePtr<int[]> p2 = p1;  // 应该编译错误
  UniquePtr<int[]> p3;
  p3 = p1;  // 应该编译错误
}
*/

// ============================================
// 主函数
// ============================================

int main() {
  std::cout << "========================================\n";
  std::cout << "       UniquePtr Test Suite\n";
  std::cout << "========================================\n\n";

  // 单对象测试
  std::cout << "--- Single Object Tests ---\n\n";
  test_default_constructor();
  test_pointer_constructor();
  test_move_constructor();
  test_move_assignment();
  test_self_move_assignment();
  test_release();
  test_reset();
  test_reset_same_pointer();
  test_swap();
  test_swap_with_nullptr();
  test_dereference();
  test_bool_conversion();
  test_make_unique();
  test_make_unique_default();
  test_with_string();
  test_nullptr_assignment();

  // 数组测试
  std::cout << "--- Array Tests ---\n\n";
  test_array_default_constructor();
  test_array_pointer_constructor();
  test_array_subscript();
  test_array_move_constructor();
  test_array_move_assignment();
  test_array_self_move_assignment();
  test_array_release();
  test_array_reset();
  test_array_swap();
  test_array_swap_with_nullptr();
  test_array_bool_conversion();
  test_array_nullptr_constructor();
  test_array_nullptr_assignment();
  test_array_primitive_types();
  test_array_const_access();
  test_make_unique_array();
  test_make_unique_array_primitive();
  test_array_large_array();

  std::cout << "========================================\n";
  std::cout << "       ALL TESTS PASSED!\n";
  std::cout << "========================================\n";

  return 0;
}