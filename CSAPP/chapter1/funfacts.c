#include <stdio.h>

int main() {
  printf("%d\n", 40000 * 40000);
  printf("%d\n", 50000 * 50000);

  printf("%lf\n", (1e20 + -1e20) + 3.14);
  printf("%lf\n", 1e20 + (-1e20 + 3.14));
}