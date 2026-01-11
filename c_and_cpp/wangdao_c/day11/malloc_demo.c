#include <stdio.h>
#include <stdlib.h>

int main() {
  // Allocate array of 5 integers
  int *arr = (int *)malloc(5 * sizeof(int));

  if (arr == NULL) {
    printf("Allocation failed!\n");
    return 1;
  }

  // Memory contains garbage - must initialize
  for (int i = 0; i < 5; i++) {
    arr[i] = i * 10;
  }

  // Print values
  for (int i = 0; i < 5; i++) {
    printf("arr[%d] = %d\n", i, arr[i]);
  }

  free(arr);
  return 0;
}