#include "consteval_demo.h"
#include <iostream>

int main() {
  // 1. constexpr function usage
  constexpr int a = square(4); // Compile-time evaluation
  int x = 4;
  int b = square(x); // Runtime evaluation

  // 2. consteval function usage
  constexpr int c = cube(3); // OK - compile time
  // int y = 3;
  // int d = cube(y);           // Error! consteval requires compile-time
  // evaluation

  // 3. Class usage
  constexpr CompileTimeExample obj(5);
  constexpr int val1 = obj.getValue();       // Can be compile-time
  constexpr int val2 = obj.getDoubleValue(); // Must be compile-time

  // Print results
  std::cout << "Square (compile-time): " << a << std::endl;
  std::cout << "Square (runtime): " << b << std::endl;
  std::cout << "Cube (compile-time): " << c << std::endl;
  std::cout << "Object value (can be compile-time): " << val1 << std::endl;
  std::cout << "Object double value (must be compile-time): " << val2
            << std::endl;

  // Check evaluation context
  std::cout << "\nEvaluation context:\n";
  constexpr auto compile_time_square = is_constant_evaluated(square(4));
  auto runtime_square = is_constant_evaluated(square(x));

  std::cout << "square(4) constant evaluated? " << compile_time_square
            << std::endl;
  std::cout << "square(x) constant evaluated? " << runtime_square << std::endl;

  return 0;
}
