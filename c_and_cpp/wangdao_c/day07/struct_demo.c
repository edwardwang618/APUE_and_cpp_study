#include <stdbool.h>
#include <stdio.h>

struct Stu {
  int number;
  char name[25];
  bool gender;
  int chinese;
  int math;
  int english;
};

int main() {
  printf("%lu\n", sizeof(struct Stu));
  return 0;
}