#include <stdio.h>

void print(const int& x) { printf("x = %d\n%p", x, &x); }

int main() {
  const int x = 3;
  // int *p = (int *)&x;
  int *p = const_cast<int *>(&x);

  *p = 4;
  printf("x = %d\n", x);
  printf("x = %d\n", *p);
  printf("x address: %p\n", p);
  printf("x address: %p\n", &x);

  print(x);
}