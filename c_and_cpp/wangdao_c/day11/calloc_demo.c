#include <stdio.h>
#include <stdlib.h>

int main() {
  // Allocate 5 integers, all initialized to 0
  int *arr = (int *)calloc(5, sizeof(int));

  if (arr == NULL) {
    printf("Allocation failed!\n");
    return 1;
  }

  // Already zero - no need to initialize
  for (int i = 0; i < 5; i++) {
    printf("arr[%d] = %d\n", i, arr[i]);
  }

  free(arr);
  return 0;
}