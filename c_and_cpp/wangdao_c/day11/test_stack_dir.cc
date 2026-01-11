#include <iostream>

void foo() {
  int x = 0;
  std::cout << &x << std::endl;
}

int main() {
  int x = 0;
  std::cout << &x << std::endl;
  foo();
}