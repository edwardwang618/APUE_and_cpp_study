#include <cassert>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

// Include your implementation
#include "my_unique_ptr_with_deleter.h"

// ============================================================================
// Test Utilities
// ============================================================================

int g_constructor_count = 0;
int g_destructor_count = 0;
int g_custom_deleter_count = 0;

void reset_counters() {
  g_constructor_count = 0;
  g_destructor_count = 0;
  g_custom_deleter_count = 0;
}

struct Tracked {
  int value;
  Tracked() : value(0) { ++g_constructor_count; }
  Tracked(int v) : value(v) { ++g_constructor_count; }
  ~Tracked() { ++g_destructor_count; }
};

struct CustomDeleter {
  void operator()(Tracked *ptr) const noexcept {
    ++g_custom_deleter_count;
    delete ptr;
  }
};

struct CustomArrayDeleter {
  void operator()(Tracked *ptr) const noexcept {
    ++g_custom_deleter_count;
    delete[] ptr;
  }
};

struct Base {
  virtual ~Base() = default;
  virtual int id() const { return 0; }
};

struct Derived : Base {
  int id() const override { return 42; }
};

// Stateful deleter for EBO testing
struct StatefulDeleter {
  int state = 123;
  void operator()(Tracked *ptr) const noexcept {
    ++g_custom_deleter_count;
    delete ptr;
  }
};

#define TEST(name)                                                             \
  void name();                                                                 \
  struct Register_##name {                                                     \
    Register_##name() { tests.push_back({#name, name}); }                      \
  } register_##name;                                                           \
  void name()

struct TestCase {
  const char *name;
  void (*func)();
};

std::vector<TestCase> tests;

// ============================================================================
// Basic Tests
// ============================================================================

TEST(test_default_constructor) {
  UniquePtr<int> p;
  assert(!p);
  assert(p.get() == nullptr);
}

TEST(test_nullptr_constructor) {
  UniquePtr<int> p(nullptr);
  assert(!p);
  assert(p.get() == nullptr);
}

TEST(test_pointer_constructor) {
  reset_counters();
  {
    UniquePtr<Tracked> p(new Tracked(42));
    assert(p);
    assert(p.get() != nullptr);
    assert(p->value == 42);
    assert((*p).value == 42);
    assert(g_constructor_count == 1);
    assert(g_destructor_count == 0);
  }
  assert(g_destructor_count == 1);
}

TEST(test_move_constructor) {
  reset_counters();
  {
    UniquePtr<Tracked> p1(new Tracked(10));
    Tracked *raw = p1.get();

    UniquePtr<Tracked> p2(std::move(p1));

    assert(!p1);
    assert(p1.get() == nullptr);
    assert(p2);
    assert(p2.get() == raw);
    assert(p2->value == 10);
    assert(g_destructor_count == 0);
  }
  assert(g_destructor_count == 1);
}

TEST(test_move_assignment) {
  reset_counters();
  {
    UniquePtr<Tracked> p1(new Tracked(1));
    UniquePtr<Tracked> p2(new Tracked(2));

    p2 = std::move(p1);

    assert(!p1);
    assert(p2);
    assert(p2->value == 1);
    assert(g_destructor_count == 1); // p2's original object deleted
  }
  assert(g_destructor_count == 2);
}

TEST(test_self_move_assignment) {
  reset_counters();
  {
    UniquePtr<Tracked> p(new Tracked(99));
    Tracked *raw = p.get();

    p = std::move(p);

    assert(p);
    assert(p.get() == raw);
    assert(p->value == 99);
    assert(g_destructor_count == 0);
  }
  assert(g_destructor_count == 1);
}

TEST(test_nullptr_assignment) {
  reset_counters();
  {
    UniquePtr<Tracked> p(new Tracked(5));
    p = nullptr;

    assert(!p);
    assert(g_destructor_count == 1);
  }
}

// ============================================================================
// Release and Reset Tests
// ============================================================================

TEST(test_release) {
  reset_counters();
  UniquePtr<Tracked> p(new Tracked(7));
  Tracked *raw = p.release();

  assert(!p);
  assert(p.get() == nullptr);
  assert(raw->value == 7);
  assert(g_destructor_count == 0);

  delete raw;
  assert(g_destructor_count == 1);
}

TEST(test_reset_nullptr) {
  reset_counters();
  {
    UniquePtr<Tracked> p(new Tracked(3));
    p.reset();

    assert(!p);
    assert(g_destructor_count == 1);
  }
}

TEST(test_reset_new_pointer) {
  reset_counters();
  {
    UniquePtr<Tracked> p(new Tracked(1));
    p.reset(new Tracked(2));

    assert(p->value == 2);
    assert(g_destructor_count == 1);
  }
  assert(g_destructor_count == 2);
}

TEST(test_reset_same_pointer) {
  reset_counters();
  {
    Tracked *raw = new Tracked(100);
    UniquePtr<Tracked> p(raw);
    p.reset(raw); // Should not delete

    assert(p);
    assert(p.get() == raw);
    assert(g_destructor_count == 0);
  }
  assert(g_destructor_count == 1);
}

// ============================================================================
// Swap Tests
// ============================================================================

TEST(test_swap) {
  reset_counters();
  {
    UniquePtr<Tracked> p1(new Tracked(1));
    UniquePtr<Tracked> p2(new Tracked(2));

    Tracked *raw1 = p1.get();
    Tracked *raw2 = p2.get();

    p1.swap(p2);

    assert(p1.get() == raw2);
    assert(p2.get() == raw1);
    assert(p1->value == 2);
    assert(p2->value == 1);
    assert(g_destructor_count == 0);
  }
  assert(g_destructor_count == 2);
}

// ============================================================================
// Custom Deleter Tests
// ============================================================================

TEST(test_custom_deleter_lvalue) {
  reset_counters();
  {
    CustomDeleter d;
    UniquePtr<Tracked, CustomDeleter> p(new Tracked(1), d);
    assert(p->value == 1);
  }
  assert(g_custom_deleter_count == 1);
  assert(g_destructor_count == 1);
}

TEST(test_custom_deleter_rvalue) {
  reset_counters();
  {
    UniquePtr<Tracked, CustomDeleter> p(new Tracked(2), CustomDeleter{});
    assert(p->value == 2);
  }
  assert(g_custom_deleter_count == 1);
  assert(g_destructor_count == 1);
}

TEST(test_get_deleter) {
  StatefulDeleter d;
  d.state = 456;
  UniquePtr<Tracked, StatefulDeleter> p(new Tracked(0), d);

  assert(p.get_deleter().state == 456);
  p.get_deleter().state = 789;
  assert(p.get_deleter().state == 789);
}

// ============================================================================
// Derived to Base Conversion Tests
// ============================================================================

TEST(test_derived_to_base_constructor) {
  UniquePtr<Derived> pd(new Derived());
  UniquePtr<Base> pb(std::move(pd));

  assert(!pd);
  assert(pb);
  assert(pb->id() == 42);
}

TEST(test_derived_to_base_assignment) {
  UniquePtr<Derived> pd(new Derived());
  UniquePtr<Base> pb;

  pb = std::move(pd);

  assert(!pd);
  assert(pb);
  assert(pb->id() == 42);
}

// ============================================================================
// Array Tests
// ============================================================================

TEST(test_array_default_constructor) {
  UniquePtr<int[]> p;
  assert(!p);
  assert(p.get() == nullptr);
}

TEST(test_array_pointer_constructor) {
  reset_counters();
  {
    UniquePtr<Tracked[]> p(new Tracked[3]);
    assert(p);
    assert(g_constructor_count == 3);
  }
  assert(g_destructor_count == 3);
}

TEST(test_array_subscript) {
  UniquePtr<int[]> p(new int[5]{10, 20, 30, 40, 50});

  assert(p[0] == 10);
  assert(p[1] == 20);
  assert(p[2] == 30);
  assert(p[3] == 40);
  assert(p[4] == 50);

  p[2] = 300;
  assert(p[2] == 300);
}

TEST(test_array_move) {
  UniquePtr<int[]> p1(new int[3]{1, 2, 3});
  int *raw = p1.get();

  UniquePtr<int[]> p2(std::move(p1));

  assert(!p1);
  assert(p2.get() == raw);
  assert(p2[0] == 1);
}

TEST(test_array_move_assignment) {
  reset_counters();
  {
    UniquePtr<Tracked[]> p1(new Tracked[2]);
    UniquePtr<Tracked[]> p2(new Tracked[3]);

    assert(g_constructor_count == 5);

    p2 = std::move(p1);

    assert(!p1);
    assert(p2);
    assert(g_destructor_count == 3); // p2's original array deleted
  }
  assert(g_destructor_count == 5);
}

TEST(test_array_custom_deleter) {
  reset_counters();
  {
    UniquePtr<Tracked[], CustomArrayDeleter> p(new Tracked[2],
                                               CustomArrayDeleter{});
    assert(p);
  }
  assert(g_custom_deleter_count == 1);
  assert(g_destructor_count == 2);
}

TEST(test_array_release) {
  reset_counters();
  UniquePtr<Tracked[]> p(new Tracked[2]);
  Tracked *raw = p.release();

  assert(!p);
  assert(g_destructor_count == 0);

  delete[] raw;
  assert(g_destructor_count == 2);
}

TEST(test_array_reset) {
  reset_counters();
  {
    UniquePtr<Tracked[]> p(new Tracked[2]);
    p.reset(new Tracked[3]);

    assert(g_constructor_count == 5);
    assert(g_destructor_count == 2);
  }
  assert(g_destructor_count == 5);
}

TEST(test_array_swap) {
  UniquePtr<int[]> p1(new int[2]{1, 2});
  UniquePtr<int[]> p2(new int[2]{3, 4});

  int *raw1 = p1.get();
  int *raw2 = p2.get();

  p1.swap(p2);

  assert(p1.get() == raw2);
  assert(p2.get() == raw1);
  assert(p1[0] == 3);
  assert(p2[0] == 1);
}

// ============================================================================
// make_unique Tests
// ============================================================================

TEST(test_make_unique_default) {
  auto p = make_unique<int>();
  assert(p);
  assert(*p == 0);
}

TEST(test_make_unique_with_args) {
  auto p = make_unique<std::string>("hello");
  assert(p);
  assert(*p == "hello");
}

TEST(test_make_unique_tracked) {
  reset_counters();
  {
    auto p = make_unique<Tracked>(42);
    assert(p);
    assert(p->value == 42);
    assert(g_constructor_count == 1);
  }
  assert(g_destructor_count == 1);
}

TEST(test_make_unique_array) {
  reset_counters();
  {
    auto p = make_unique<Tracked[]>(5);
    assert(p);
    assert(g_constructor_count == 5);

    for (size_t i = 0; i < 5; ++i) {
      assert(p[i].value == 0); // Value initialized
    }
  }
  assert(g_destructor_count == 5);
}

TEST(test_make_unique_array_zero_size) {
  auto p = make_unique<int[]>(0);
  assert(p); // Valid but points to zero-length array
}

// ============================================================================
// EBO (Empty Base Optimization) Test
// ============================================================================

TEST(test_ebo) {
  // With [[no_unique_address]], UniquePtr with empty deleter
  // should be the same size as a raw pointer
  static_assert(sizeof(UniquePtr<int>) == sizeof(int *),
                "EBO not working for default deleter");

  // With stateful deleter, size should be larger
  static_assert(sizeof(UniquePtr<int, StatefulDeleter>) > sizeof(int *),
                "Stateful deleter should increase size");
}

// ============================================================================
// Bool Conversion Tests
// ============================================================================

TEST(test_bool_conversion) {
  UniquePtr<int> empty;
  UniquePtr<int> valid(new int(5));

  assert(!empty);
  assert(valid);

  if (empty) {
    assert(false && "empty should be false");
  }

  if (!valid) {
    assert(false && "valid should be true");
  }
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(test_multiple_moves) {
  reset_counters();
  {
    UniquePtr<Tracked> p1(new Tracked(1));
    UniquePtr<Tracked> p2(std::move(p1));
    UniquePtr<Tracked> p3(std::move(p2));
    UniquePtr<Tracked> p4;
    p4 = std::move(p3);

    assert(!p1);
    assert(!p2);
    assert(!p3);
    assert(p4);
    assert(p4->value == 1);
    assert(g_destructor_count == 0);
  }
  assert(g_destructor_count == 1);
}

TEST(test_nullptr_operations) {
  UniquePtr<int> p;

  // These should not crash
  p.reset();
  p.reset(nullptr);
  assert(p.release() == nullptr);

  UniquePtr<int> p2;
  p.swap(p2);
  assert(!p);
  assert(!p2);
}

// ============================================================================
// Main
// ============================================================================

int main() {
  int passed = 0;
  int failed = 0;

  std::cout << "Running " << tests.size() << " tests...\n";
  std::cout << std::string(60, '=') << "\n";

  for (const auto &test : tests) {
    reset_counters();
    try {
      test.func();
      std::cout << "[PASS] " << test.name << "\n";
      ++passed;
    } catch (const std::exception &e) {
      std::cout << "[FAIL] " << test.name << ": " << e.what() << "\n";
      ++failed;
    } catch (...) {
      std::cout << "[FAIL] " << test.name << ": unknown exception\n";
      ++failed;
    }
  }

  std::cout << std::string(60, '=') << "\n";
  std::cout << "Results: " << passed << " passed, " << failed << " failed\n";

  if (failed == 0) {
    std::cout << "\n*** All tests passed! ***\n";
  }

  return failed > 0 ? 1 : 0;
}