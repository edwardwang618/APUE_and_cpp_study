#include <stdio.h>

#define INCREMENT(x) ((x) + 1)

int main() {
  int a = 5;
  int b = INCREMENT(a++);
  // a is incremented twice due to re-evaluation
  printf("a = %d, b = %d\n", a, b);
  return 0;
}