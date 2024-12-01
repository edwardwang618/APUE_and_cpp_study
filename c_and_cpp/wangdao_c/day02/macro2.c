#include <stdio.h>

int main() {
  int x = 10;
#ifdef DEBUG
  printf("Debug: x = %d\n", x);
#endif
  // Rest of the code
  return 0;
}

// gcc -DDEBUG macro2.c -o macro2