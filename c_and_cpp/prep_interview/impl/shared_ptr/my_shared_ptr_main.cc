#include "my_shared_ptr.h"
#include <cassert>
#include <iostream>

int main() {
  std::cout << "=== Test 1: Basic construction ===" << std::endl;
  {
    SharedPtr<int> p(new int(42));
    assert(p.use_count() == 1);
    assert(*p == 42);
    assert(p.get() != nullptr);
    std::cout << "PASSED" << std::endl;
  }

  std::cout << "=== Test 2: Copy constructor ===" << std::endl;
  {
    SharedPtr<int> p1(new int(10));
    std::cout << p1.use_count() << std::endl;
    SharedPtr<int> p2 = p1;
    assert(p1.use_count() == 2);
    assert(p2.use_count() == 2);
    assert(p1.get() == p2.get());
    assert(*p1 == 10);
    assert(*p2 == 10);
    std::cout << "PASSED" << std::endl;
  }

  std::cout << "=== Test 3: Copy assignment ===" << std::endl;
  {
    SharedPtr<int> p1(new int(1));
    SharedPtr<int> p2(new int(2));
    p1 = p2;
    assert(p1.use_count() == 2);
    assert(p2.use_count() == 2);
    assert(*p1 == 2);
    assert(*p2 == 2);
    std::cout << "PASSED" << std::endl;
  }

  std::cout << "=== Test 4: Self assignment ===" << std::endl;
  {
    SharedPtr<int> p(new int(5));
    p = p;
    assert(p.use_count() == 1);
    assert(*p == 5);
    std::cout << "PASSED" << std::endl;
  }

  std::cout << "=== Test 5: Empty pointer ===" << std::endl;
  {
    SharedPtr<int> p;
    assert(p.get() == nullptr);
    assert(p.use_count() == 0);
    assert(!p);
    std::cout << "PASSED" << std::endl;
  }

  std::cout << "=== Test 6: Multiple copies ===" << std::endl;
  {
    SharedPtr<int> p1(new int(100));
    SharedPtr<int> p2 = p1;
    SharedPtr<int> p3 = p2;
    SharedPtr<int> p4 = p1;
    assert(p1.use_count() == 4);
    assert(p2.use_count() == 4);
    assert(p3.use_count() == 4);
    assert(p4.use_count() == 4);
    std::cout << "PASSED" << std::endl;
  }

  std::cout << "=== Test 7: Destructor decrements count ===" << std::endl;
  {
    SharedPtr<int> p1(new int(50));
    {
      SharedPtr<int> p2 = p1;
      assert(p1.use_count() == 2);
    }
    assert(p1.use_count() == 1);
    std::cout << "PASSED" << std::endl;
  }

  std::cout << "=== Test 8: operator-> ===" << std::endl;
  {
    struct Point {
      int x;
      int y;
    };
    SharedPtr<Point> p(new Point{3, 4});
    assert(p->x == 3);
    assert(p->y == 4);
    p->x = 10;
    assert(p->x == 10);
    std::cout << "PASSED" << std::endl;
  }

  std::cout << "=== Test 9: swap ===" << std::endl;
  {
    SharedPtr<int> p1(new int(1));
    SharedPtr<int> p2(new int(2));

    p1.swap(p2);

    assert(*p1 == 2);
    assert(*p2 == 1);
    assert(p1.use_count() == 1);
    assert(p2.use_count() == 1);
    std::cout << "PASSED" << std::endl;
  }

  std::cout << "=== Test 10: swap with empty ===" << std::endl;
  {
    SharedPtr<int> p1(new int(42));
    SharedPtr<int> p2;

    p1.swap(p2);

    assert(!p1);
    assert(*p2 == 42);
    std::cout << "PASSED" << std::endl;
  }

  std::cout << "=== Test 11: Move constructor ===" << std::endl;
  {
    SharedPtr<int> p1(new int(99));
    SharedPtr<int> p2(std::move(p1));
    assert(!p1);
    assert(p1.use_count() == 0);
    assert(*p2 == 99);
    assert(p2.use_count() == 1);
    std::cout << "PASSED" << std::endl;
  }

  std::cout << "=== Test 12: Move assignment ===" << std::endl;
  {
    SharedPtr<int> p1(new int(11));
    SharedPtr<int> p2(new int(22));
    p2 = std::move(p1);
    assert(!p1);
    assert(*p2 == 11);
    assert(p2.use_count() == 1);
    std::cout << "PASSED" << std::endl;
  }

  std::cout << "=== Test 13: reset ===" << std::endl;
  {
    SharedPtr<int> p1(new int(100));
    SharedPtr<int> p2 = p1;
    assert(p1.use_count() == 2);

    p1.reset(new int(200));
    assert(p1.use_count() == 1);
    assert(p2.use_count() == 1);
    assert(*p1 == 200);
    assert(*p2 == 100);

    p1.reset();
    assert(!p1);
    assert(p1.use_count() == 0);
    std::cout << "PASSED" << std::endl;
  }

  std::cout << "=== Test 14: nullptr construction ===" << std::endl;
  {
    SharedPtr<int> p(nullptr);
    assert(!p);
    assert(p.use_count() == 0);
    assert(p.get() == nullptr);
    std::cout << "PASSED" << std::endl;
  }

  std::cout << "=== Test 15: WeakPtr basic ===" << std::endl;
  {
    SharedPtr<int> sp(new int(42));
    WeakPtr<int> wp(sp);

    assert(wp.use_count() == 1);
    assert(!wp.expired());

    auto locked = wp.lock();
    assert(locked);
    assert(*locked == 42);
    assert(sp.use_count() == 2);
    std::cout << "PASSED" << std::endl;
  }

  std::cout << "=== Test 16: WeakPtr expires ===" << std::endl;
  {
    WeakPtr<int> wp;
    {
      SharedPtr<int> sp(new int(123));
      wp = sp;
      assert(!wp.expired());
      assert(wp.use_count() == 1);
    }
    assert(wp.expired());
    assert(wp.use_count() == 0);

    auto locked = wp.lock();
    assert(!locked);
    std::cout << "PASSED" << std::endl;
  }

  std::cout << "=== Test 17: WeakPtr copy ===" << std::endl;
  {
    SharedPtr<int> sp(new int(77));
    WeakPtr<int> wp1(sp);
    WeakPtr<int> wp2(wp1);
    WeakPtr<int> wp3;
    wp3 = wp2;

    assert(wp1.use_count() == 1);
    assert(wp2.use_count() == 1);
    assert(wp3.use_count() == 1);

    auto l1 = wp1.lock();
    auto l2 = wp2.lock();
    auto l3 = wp3.lock();
    assert(sp.use_count() == 4);
    assert(*l1 == 77);
    assert(*l2 == 77);
    assert(*l3 == 77);
    std::cout << "PASSED" << std::endl;
  }

  std::cout << "=== Test 18: WeakPtr doesn't prevent deletion ===" << std::endl;
  {
    int *raw = new int(999);
    WeakPtr<int> wp;
    {
      SharedPtr<int> sp(raw);
      wp = sp;
      assert(sp.use_count() == 1); // WeakPtr 不增加 strong_count
    }
    // sp 销毁，对象被删除
    assert(wp.expired());
    assert(!wp.lock());
    std::cout << "PASSED" << std::endl;
  }

  std::cout << "=== Test 19: Multiple WeakPtr ===" << std::endl;
  {
    SharedPtr<int> sp(new int(55));
    WeakPtr<int> wp1(sp);
    WeakPtr<int> wp2(sp);
    WeakPtr<int> wp3(sp);

    assert(sp.use_count() == 1); // 只有一个 SharedPtr

    sp.reset();
    assert(wp1.expired());
    assert(wp2.expired());
    assert(wp3.expired());
    std::cout << "PASSED" << std::endl;
  }

  std::cout << "=== Test 20: Empty WeakPtr ===" << std::endl;
  {
    WeakPtr<int> wp;
    assert(wp.expired());
    assert(wp.use_count() == 0);
    assert(!wp.lock());
    std::cout << "PASSED" << std::endl;
  }

  std::cout << "\nAll tests passed!" << std::endl;
  return 0;
}