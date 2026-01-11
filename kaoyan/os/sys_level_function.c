#include <stdio.h>
#include <unistd.h>

int main() {
  write(STDOUT_FILENO, "hello world", 13);
  _exit(0);
}