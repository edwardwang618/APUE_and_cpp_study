#include <stdio.h>

// Define the add function outside of main
void add(int a, int b) { printf("a + b = %d\n", a + b); }

int main() {
  printf("Hello, World!\n");

  // Define a function pointer and assign it to the add function
  void (*add_ptr)(int, int) = add;

  // Call the add function using the function pointer
  add_ptr(1, 2);

  return 0;
}