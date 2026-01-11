#include <stdio.h>

int main(int argc, char* argv[]) {
  printf("argc = %d\n", argc);
  puts("=======================================");
  for (char** arg = argv; *arg; arg++) puts(*arg);
  puts("=======================================");
  for (int i = 0; i < argc; i++) puts(argv[i]);
}