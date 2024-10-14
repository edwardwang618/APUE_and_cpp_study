#include <stdio.h>

#define LOG(format, ...) fprintf(stderr, format, __VA_ARGS__)

#define TO_STRING(x) #x
// #define TO_STRING(...) #__VA_ARGS__

#define CONCAT(a, b) a##b

int main() {
  int errorCode = 5;
  LOG("Error code: %d\n", errorCode);

  printf("%s\n", TO_STRING(HelloWorld !));
  // printf("%s\n", TO_STRING(Hello, World!));
  // Output: Hello, World!

  int xy = 10;
  int x1 = 20;
  int x = 5;
  printf("%d\n", TO_STRING(x));
  printf("%d\n", CONCAT(x, y));    // Accesses variable 'xy'
  printf("%d\n", CONCAT(x, 1));    // Accesses variable 'x1'

  return 0;
}