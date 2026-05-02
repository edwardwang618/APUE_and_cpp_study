#include <stdio.h>

void foo(const char **p) { *p = "Hi"; }

int main() {
  const char *s = "Hello!";
  foo(&s);
  
  puts(s);
}