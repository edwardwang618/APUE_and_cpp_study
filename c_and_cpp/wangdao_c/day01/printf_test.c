#include <stdio.h>
#include <string.h>

int main() {
  // Basic types
  printf("Integer: %d\n", 42);
  printf("Float: %f\n", 3.14159);
  printf("Char: %c\n", 'A');
  int res = printf("String: %s\n", "Hello");
  printf("%d\n", res);
  printf("%lu\n", strlen("String: Hello\n"));
  return 0;
}