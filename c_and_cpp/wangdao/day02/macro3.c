#include <stdio.h>
#include <stdlib.h>

#define LOG_ERROR(msg)                   \
  do {                                   \
    fprintf(stderr, "Error: %s\n", msg); \
    exit(EXIT_FAILURE);                  \
  } while (0)

int main() {
  // Some code
  int error_condition = 1;
  if (error_condition) {
    LOG_ERROR("An unexpected error occurred.");
  }
  // Rest of the code
  return 0;
}