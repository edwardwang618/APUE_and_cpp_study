#include <cstdio>
#include <iostream>

void my_printf(const char *s) { std::cout << s; }

template <typename T, typename... Args>
void my_printf(const char *s, T v, Args... args) {
  while (*s) {
    if (*s == '{' && *(s + 1) == '}') {
      std::cout << v;
      my_printf(s + 2, args...);
      return;
    } else {
      putchar(*s++);
    }
  }
}

int main() {
  my_printf("My name is {}, age is {}\n", "Andy", 55);
  my_printf("Hello {} {}", 1, 2, 3);
}