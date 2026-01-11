#include "my_pair.h"
#include <cassert>
#include <iostream>
#include <string>
#include <utility>

// Your Pair implementation goes here
// template <typename T1, typename T2> struct Pair { ... };

// Helper class to test move semantics
struct MoveOnly {
  int value;
  MoveOnly(int v = 0) : value(v) {}
  MoveOnly(const MoveOnly &) = delete;
  MoveOnly &operator=(const MoveOnly &) = delete;
  MoveOnly(MoveOnly &&other) noexcept : value(other.value) { other.value = -1; }
  MoveOnly &operator=(MoveOnly &&other) noexcept {
    value = other.value;
    other.value = -1;
    return *this;
  }
};

// Helper class to count copies and moves
struct Counter {
  static int copies;
  static int moves;
  int value;
  Counter(int v = 0) : value(v) {}
  Counter(const Counter &other) : value(other.value) { ++copies; }
  Counter(Counter &&other) noexcept : value(other.value) {
    ++moves;
    other.value = -1;
  }
  Counter &operator=(const Counter &other) {
    value = other.value;
    ++copies;
    return *this;
  }
  Counter &operator=(Counter &&other) noexcept {
    value = other.value;
    ++moves;
    other.value = -1;
    return *this;
  }
  static void reset() { copies = moves = 0; }
};
int Counter::copies = 0;
int Counter::moves = 0;

int main() {
  std::cout << "=== Test 1: Default Constructor ===" << std::endl;
  {
    Pair<int, double> p1;
    assert(p1.first == 0);
    assert(p1.second == 0.0);

    Pair<std::string, int> p2;
    assert(p2.first == "");
    assert(p2.second == 0);
    std::cout << "PASSED\n";
  }

  std::cout << "=== Test 2: Value Constructor ===" << std::endl;
  {
    Pair<int, std::string> p(42, "hello");
    assert(p.first == 42);
    assert(p.second == "hello");

    int a = 10;
    std::string b = "world";
    Pair<int, std::string> p2(a, b);
    assert(p2.first == 10);
    assert(p2.second == "world");
    std::cout << "PASSED\n";
  }

  std::cout << "=== Test 3: Copy Constructor ===" << std::endl;
  {
    Pair<int, std::string> p1(1, "test");
    Pair<int, std::string> p2(p1);
    assert(p2.first == 1);
    assert(p2.second == "test");

    p1.first = 999;
    assert(p2.first == 1); // Deep copy
    std::cout << "PASSED\n";
  }

  std::cout << "=== Test 4: Move Constructor ===" << std::endl;
  {
    Pair<int, std::string> p1(42, "move me");
    Pair<int, std::string> p2(std::move(p1));
    assert(p2.first == 42);
    assert(p2.second == "move me");
    assert(p1.second.empty()); // Moved-from state
    std::cout << "PASSED\n";
  }

  std::cout << "=== Test 5: Move-Only Types ===" << std::endl;
  {
    Pair<MoveOnly, MoveOnly> p(MoveOnly(10), MoveOnly(20));
    assert(p.first.value == 10);
    assert(p.second.value == 20);

    Pair<MoveOnly, MoveOnly> p2(std::move(p));
    assert(p2.first.value == 10);
    assert(p.first.value == -1); // Moved
    std::cout << "PASSED\n";
  }

  std::cout << "=== Test 6: Converting Constructor ===" << std::endl;
  {
    Pair<int, int> p1(1, 2);
    Pair<double, double> p2(p1); // int -> double
    assert(p2.first == 1.0);
    assert(p2.second == 2.0);

    Pair<int, const char *> p3(10, "hello");
    Pair<long, std::string> p4(p3); // int->long, const char*->string
    assert(p4.first == 10L);
    assert(p4.second == "hello");
    std::cout << "PASSED\n";
  }

  std::cout << "=== Test 7: Copy Assignment ===" << std::endl;
  {
    Pair<int, std::string> p1(1, "one");
    Pair<int, std::string> p2(2, "two");
    p2 = p1;
    assert(p2.first == 1);
    assert(p2.second == "one");

    // Self-assignment
    p1 = p1;
    assert(p1.first == 1);
    std::cout << "PASSED\n";
  }

  std::cout << "=== Test 8: Move Assignment ===" << std::endl;
  {
    Pair<int, std::string> p1(1, "movable");
    Pair<int, std::string> p2(2, "target");
    p2 = std::move(p1);
    assert(p2.first == 1);
    assert(p2.second == "movable");
    std::cout << "PASSED\n";
  }

  std::cout << "=== Test 9: Converting Assignment ===" << std::endl;
  {
    Pair<int, int> p1(100, 200);
    Pair<double, double> p2;
    p2 = p1;
    assert(p2.first == 100.0);
    assert(p2.second == 200.0);
    std::cout << "PASSED\n";
  }

  std::cout << "=== Test 10: swap ===" << std::endl;
  {
    Pair<int, std::string> p1(1, "aaa");
    Pair<int, std::string> p2(2, "bbb");
    p1.swap(p2);
    assert(p1.first == 2 && p1.second == "bbb");
    assert(p2.first == 1 && p2.second == "aaa");

    swap(p1, p2);
    assert(p1.first == 1 && p1.second == "aaa");
    std::cout << "PASSED\n";
  }

  std::cout << "=== Test 11: Comparison ===" << std::endl;
  {
    Pair<int, int> p1(1, 2);
    Pair<int, int> p2(1, 2);
    Pair<int, int> p3(1, 3);
    Pair<int, int> p4(2, 1);

    assert(p1 == p2);
    assert(!(p1 == p3));

    assert(p1 < p3); // first equal, compare second
    assert(p1 < p4); // first differs
    assert(p3 < p4);

    assert(p1 <= p2);
    assert(p1 >= p2);
    assert(p4 > p1);
    std::cout << "PASSED\n";
  }

  std::cout << "=== Test 12: make_pair ===" << std::endl;
  {
    auto p1 = ::make_pair(42, 3.14);
    static_assert(std::is_same_v<decltype(p1.first), int>);
    static_assert(std::is_same_v<decltype(p1.second), double>);

    std::string s = "hello";
    auto p2 = ::make_pair(s, 100); // Should copy, not reference
    static_assert(std::is_same_v<decltype(p2.first), std::string>);

    auto p3 = ::make_pair(std::move(s), 1);
    assert(s.empty()); // s was moved
    std::cout << "PASSED\n";
  }

  std::cout << "=== Test 13: Perfect Forwarding ===" << std::endl;
  {
    Counter::reset();
    Counter c(5);

    Pair<Counter, Counter> p1(c, c); // Should copy
    assert(Counter::copies == 2);

    Counter::reset();
    Pair<Counter, Counter> p2(std::move(c), Counter(10)); // Should move
    assert(Counter::moves == 2);
    assert(Counter::copies == 0);
    std::cout << "PASSED\n";
  }

  std::cout << "=== Test 14: Nested Pair ===" << std::endl;
  {
    Pair<int, Pair<int, int>> nested(1, Pair<int, int>(2, 3));
    assert(nested.first == 1);
    assert(nested.second.first == 2);
    assert(nested.second.second == 3);
    std::cout << "PASSED\n";
  }

  std::cout << "\n=== ALL TESTS PASSED ===" << std::endl;
  return 0;
}