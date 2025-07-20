#pragma once
#include <type_traits>

// 1. constexpr function - can be evaluated at compile time OR runtime
constexpr int square(int n) { return n * n; }

// 2. consteval function - MUST be evaluated at compile time
consteval int cube(int n) { return n * n * n; }

// Example usage in class
class CompileTimeExample {
public:
  // constexpr constructor allows compile-time instantiation
  constexpr CompileTimeExample(int n) : value(n) {}

  // constexpr member function - can run at compile time or runtime
  constexpr int getValue() const { return value; }

  // consteval member function - must run at compile time
  consteval int getDoubleValue() const { return value * 2; }

private:
  int value;
};

// Helper to test if an expression is evaluated at compile time
template <typename T> constexpr bool is_constant_evaluated(T) {
  return std::is_constant_evaluated();
}
