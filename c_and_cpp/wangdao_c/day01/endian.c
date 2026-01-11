#include <stdio.h>

int main() {
  int x = 0x1;
  char *p = (char *)&x;
  if (*p == 1)
    printf("little endian\n");
  else
    printf("big endian\n");

  for (int i = 0; i < sizeof(int); i++)
    printf("%p: %04d\n", p + i, *(p + i));
}