#include <iostream>

template <typename T> T max(T a, T b) { return (a > b) ? a : b; }

int main() {
  std::cout << max(3, 5) << "\n";       // int 版本
  std::cout << max(3.14, 2.71) << "\n"; // double 版本
}