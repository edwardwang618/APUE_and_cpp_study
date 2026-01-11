#include <stdio.h>
#include <stdlib.h>

int main() {
  // Start with 3 integers
  int *arr = (int *)malloc(3 * sizeof(int));

  arr[0] = 10;
  arr[1] = 20;
  arr[2] = 30;

  printf("Before realloc:\n");
  for (int i = 0; i < 3; i++) {
    printf("arr[%d] = %d\n", i, arr[i]);
  }

  // Expand to 5 integers
  int *temp = (int *)realloc(arr, 5 * sizeof(int));

  if (temp == NULL) {
    printf("Realloc failed!\n");
    free(arr);
    return 1;
  }

  arr = temp;

  // Old data preserved, new elements are garbage
  arr[3] = 40;
  arr[4] = 50;

  printf("\nAfter realloc:\n");
  for (int i = 0; i < 5; i++) {
    printf("arr[%d] = %d\n", i, arr[i]);
  }

  free(arr);
  return 0;
}