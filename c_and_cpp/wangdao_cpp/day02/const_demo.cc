#include <cstdlib>
#include <iostream>

void display(int *p) {
  *p = 10;
  std::cout << *p << std::endl;
}

int main() {
  int *p = static_cast<int*>(malloc(sizeof(int)));
  const volatile int a = 1;
  display(const_cast<int*>(&a));
  std::cout << a << std::endl;
}