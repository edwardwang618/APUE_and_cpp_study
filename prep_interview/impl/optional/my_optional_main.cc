#include "my_optional.h"
#include <cassert>
#include <iostream>
#include <string>

// 你的 Optional 代码放这里

struct Tracked {
  static int count;
  int value;

  Tracked() : value(0) { count++; }
  Tracked(int v) : value(v) { count++; }
  Tracked(const Tracked &other) : value(other.value) { count++; }
  Tracked(Tracked &&other) : value(other.value) { count++; }
  ~Tracked() { count--; }
};
int Tracked::count = 0;

void test_basic() {
  Optional<int> empty;
  assert(!empty.has_value());

  Optional<int> full(42);
  assert(full.has_value());
  assert(full.value() == 42);
  assert(*full == 42);

  std::cout << "test_basic passed\n";
}

void test_copy_move() {
  Optional<int> a(10);

  Optional<int> b(a); // copy
  assert(b.has_value() && *b == 10);

  Optional<int> c(std::move(a)); // move
  assert(c.has_value() && *c == 10);

  Optional<int> d;
  d = b; // copy assign
  assert(d.has_value() && *d == 10);

  Optional<int> e;
  e = std::move(b); // move assign
  assert(e.has_value() && *e == 10);

  std::cout << "test_copy_move passed\n";
}

void test_reset_emplace() {
  Optional<int> opt(5);
  opt.reset();
  assert(!opt.has_value());

  opt.emplace(100);
  assert(opt.has_value() && *opt == 100);

  opt.emplace(200); // emplace over existing
  assert(*opt == 200);

  std::cout << "test_reset_emplace passed\n";
}

void test_no_leak() {
  {
    Optional<Tracked> a(Tracked(1));
    Optional<Tracked> b(a);
    Optional<Tracked> c(std::move(a));
    b.reset();
    c.emplace(42);
  }
  assert(Tracked::count == 0);
  std::cout << "test_no_leak passed\n";
}

void test_empty_operations() {
  Optional<Tracked> empty;
  Optional<Tracked> copy(empty);
  assert(!copy.has_value());

  Optional<Tracked> moved(std::move(empty));
  assert(!moved.has_value());

  Optional<Tracked> a;
  a = Optional<Tracked>();
  assert(!a.has_value());

  assert(Tracked::count == 0);
  std::cout << "test_empty_operations passed\n";
}

int main() {
  test_basic();
  test_copy_move();
  test_reset_emplace();
  test_no_leak();
  test_empty_operations();

  std::cout << "\nAll tests passed!\n";
  return 0;
}