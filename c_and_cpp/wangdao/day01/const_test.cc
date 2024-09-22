#include <stdio.h>

int main() {
  const int x = 3;
  // int *p = (int *)&x;
  int *p = const_cast<int *>(&x);

  *p = 4;
  printf("x = %d\n", x);
  printf("x = %d\n", *p);
  printf("x address: %p\n", p);
  printf("x address: %p\n", &x);
}