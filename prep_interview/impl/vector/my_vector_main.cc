#include <cassert>
#include <iostream>
#include <string>
#include "my_vector.h"

// 把你的 Vector 代码放在这里，或者 #include "vector.hpp"

// 用于追踪构造/析构的测试类
struct Tracked {
  static int count;
  int value;

  Tracked() : value(0) { count++; }
  Tracked(int v) : value(v) { count++; }
  Tracked(const Tracked &other) : value(other.value) { count++; }
  Tracked(Tracked &&other) : value(other.value) {
    other.value = -1;
    count++;
  }
  Tracked &operator=(const Tracked &other) {
    value = other.value;
    return *this;
  }
  Tracked &operator=(Tracked &&other) {
    value = other.value;
    other.value = -1;
    return *this;
  }
  ~Tracked() { count--; }
};
int Tracked::count = 0;

void test_basic() {
  Vector<int> v;
  assert(v.size() == 0);
  assert(v.empty());

  v.push_back(1);
  v.push_back(2);
  v.push_back(3);
  assert(v.size() == 3);
  assert(v[0] == 1 && v[1] == 2 && v[2] == 3);

  v.pop_back();
  assert(v.size() == 2);

  std::cout << "test_basic passed\n";
}

void test_copy_move() {
  Vector<int> v1;
  v1.push_back(1);
  v1.push_back(2);

  Vector<int> v2(v1); // copy
  assert(v2.size() == 2);
  assert(v2[0] == 1 && v2[1] == 2);

  Vector<int> v3(std::move(v1)); // move
  assert(v3.size() == 2);
  assert(v1.size() == 0);

  v2 = v3; // copy assign
  assert(v2.size() == 2);

  v1 = std::move(v3); // move assign
  assert(v1.size() == 2);
  assert(v3.size() == 0);

  std::cout << "test_copy_move passed\n";
}

void test_erase() {
  Vector<int> v;
  for (int i = 0; i < 5; i++)
    v.push_back(i); // {0,1,2,3,4}

  v.erase(v.begin() + 2); // {0,1,3,4}
  assert(v.size() == 4);
  assert(v[2] == 3);

  v.erase(v.begin(), v.begin() + 2); // {3,4}
  assert(v.size() == 2);
  assert(v[0] == 3 && v[1] == 4);

  std::cout << "test_erase passed\n";
}

void test_insert() {
  Vector<int> v;
  v.insert(v.begin(), 1); // {1}
  assert(v.size() == 1 && v[0] == 1);

  v.insert(v.begin(), 0); // {0,1}
  assert(v.size() == 2 && v[0] == 0);

  v.insert(v.end(), 2); // {0,1,2}
  assert(v.size() == 3 && v[2] == 2);

  v.insert(v.begin() + 1, 3, 9); // {0,9,9,9,1,2}
  assert(v.size() == 6);
  assert(v[0] == 0 && v[1] == 9 && v[2] == 9 && v[3] == 9 && v[4] == 1);

  // 空 vector 插入多个
  Vector<int> v2;
  v2.insert(v2.begin(), 3, 5);
  assert(v2.size() == 3);
  assert(v2[0] == 5 && v2[1] == 5 && v2[2] == 5);

  std::cout << "test_insert passed\n";
}

void test_no_leak() {
  {
    Vector<Tracked> v;
    v.push_back(Tracked(1));
    v.push_back(Tracked(2));
    v.emplace_back(3);
    v.pop_back();
    v.insert(v.begin(), Tracked(0));
    v.erase(v.begin() + 1);
  }
  assert(Tracked::count == 0);
  std::cout << "test_no_leak passed\n";
}

void test_resize_reserve() {
  Vector<Tracked> v;
  v.reserve(100);
  assert(v.capacity() >= 100);
  assert(Tracked::count == 0);

  v.resize(10);
  assert(v.size() == 10);
  assert(Tracked::count == 10);

  v.resize(5);
  assert(v.size() == 5);
  assert(Tracked::count == 5);

  v.clear();
  assert(v.size() == 0);
  assert(Tracked::count == 0);

  std::cout << "test_resize_reserve passed\n";
}

int main() {
  test_basic();
  test_copy_move();
  test_erase();
  test_insert();
  test_no_leak();
  test_resize_reserve();

  std::cout << "\nAll tests passed!\n";
  return 0;
}