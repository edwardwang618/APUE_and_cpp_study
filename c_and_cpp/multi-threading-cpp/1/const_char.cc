#include <stdio.h>

void func(const char **p) { *p = "Jane Street"; }

void func2(char const **q) { *q = "Hudson River Trading"; }

int main() {
  const char *p = "Hudson River Trading";
  func(&p);

  char const *q = "Jane Street";
  func2(&q);

  printf("%s\n", p);
  printf("%s\n", q);
}