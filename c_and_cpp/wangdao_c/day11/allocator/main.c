#include "my_allocator.h"
#include <stdio.h>
#include <string.h>

// Example struct
typedef struct {
  int id;
  char name[50];
  float score;
} Student;

void test_basic_malloc(void) {
  printf("=== Test 1: Basic malloc ===\n");

  int *arr = (int *)my_malloc(5 * sizeof(int));

  for (int i = 0; i < 5; i++) {
    arr[i] = i * 10;
  }

  printf("Array: ");
  for (int i = 0; i < 5; i++) {
    printf("%d ", arr[i]);
  }
  printf("\n");

  my_free(arr);
  printf("Freed successfully\n\n");
}

void test_calloc(void) {
  printf("=== Test 2: calloc (zero-initialized) ===\n");

  int *arr = (int *)my_calloc(5, sizeof(int));

  printf("Calloc array (should be zeros): ");
  for (int i = 0; i < 5; i++) {
    printf("%d ", arr[i]);
  }
  printf("\n");

  my_free(arr);
  printf("\n");
}

void test_realloc(void) {
  printf("=== Test 3: realloc ===\n");

  int *arr = (int *)my_malloc(3 * sizeof(int));
  arr[0] = 100;
  arr[1] = 200;
  arr[2] = 300;

  printf("Before realloc: ");
  for (int i = 0; i < 3; i++) {
    printf("%d ", arr[i]);
  }
  printf("\n");

  // Expand to 6 integers
  arr = (int *)my_realloc(arr, 6 * sizeof(int));
  arr[3] = 400;
  arr[4] = 500;
  arr[5] = 600;

  printf("After realloc:  ");
  for (int i = 0; i < 6; i++) {
    printf("%d ", arr[i]);
  }
  printf("\n");

  my_free(arr);
  printf("\n");
}

void test_string(void) {
  printf("=== Test 4: Dynamic string ===\n");

  char *str = (char *)my_malloc(20);
  strcpy(str, "Hello");
  printf("String: %s\n", str);

  str = (char *)my_realloc(str, 50);
  strcat(str, ", World!");
  printf("Extended: %s\n", str);

  my_free(str);
  printf("\n");
}

void test_struct(void) {
  printf("=== Test 5: Struct allocation ===\n");

  Student *s = (Student *)my_malloc(sizeof(Student));
  s->id = 1;
  strcpy(s->name, "Alice");
  s->score = 95.5;

  printf("Student: ID=%d, Name=%s, Score=%.1f\n", s->id, s->name, s->score);

  my_free(s);
  printf("\n");
}

void test_array_of_structs(void) {
  printf("=== Test 6: Array of structs ===\n");

  int n = 3;
  Student *students = (Student *)my_calloc(n, sizeof(Student));

  for (int i = 0; i < n; i++) {
    students[i].id = i + 1;
    sprintf(students[i].name, "Student_%d", i + 1);
    students[i].score = 80.0 + i * 5;
  }

  for (int i = 0; i < n; i++) {
    printf("  ID=%d, Name=%s, Score=%.1f\n", students[i].id, students[i].name,
           students[i].score);
  }

  my_free(students);
  printf("\n");
}

void test_2d_array(void) {
  printf("=== Test 7: 2D array ===\n");

  int rows = 3;
  int cols = 4;

  // Allocate row pointers
  int **matrix = (int **)my_malloc(rows * sizeof(int *));

  // Allocate each row
  for (int i = 0; i < rows; i++) {
    matrix[i] = (int *)my_malloc(cols * sizeof(int));
  }

  // Fill matrix
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      matrix[i][j] = i * cols + j;
    }
  }

  // Print matrix
  for (int i = 0; i < rows; i++) {
    printf("  ");
    for (int j = 0; j < cols; j++) {
      printf("%2d ", matrix[i][j]);
    }
    printf("\n");
  }

  // Free each row
  for (int i = 0; i < rows; i++) {
    my_free(matrix[i]);
  }
  my_free(matrix);
  printf("\n");
}

void test_multiple_allocations(void) {
  printf("=== Test 8: Multiple allocations ===\n");

  int *a = (int *)my_malloc(100);
  int *b = (int *)my_malloc(200);
  int *c = (int *)my_malloc(300);

  printf("Allocated a, b, c\n");
  printf("Used: %zu bytes, Free: %zu bytes\n", my_heap_used(), my_heap_free());

  my_free(b);
  printf("\nFreed b\n");
  printf("Used: %zu bytes, Free: %zu bytes\n", my_heap_used(), my_heap_free());

  int *d = (int *)my_malloc(150);
  printf("\nAllocated d (should reuse b's space)\n");
  printf("Used: %zu bytes, Free: %zu bytes\n", my_heap_used(), my_heap_free());

  my_free(a);
  my_free(c);
  my_free(d);
  printf("\nFreed all\n");
  printf("Used: %zu bytes, Free: %zu bytes\n", my_heap_used(), my_heap_free());

  printf("\n");
}

void test_growing_array(void) {
  printf("=== Test 9: Growing array (like vector) ===\n");

  int capacity = 2;
  int size = 0;
  int *arr = (int *)my_malloc(capacity * sizeof(int));

  for (int i = 0; i < 10; i++) {
    if (size >= capacity) {
      capacity *= 2;
      arr = (int *)my_realloc(arr, capacity * sizeof(int));
      printf("  Grew to capacity %d\n", capacity);
    }
    arr[size++] = i * i;
  }

  printf("Final array: ");
  for (int i = 0; i < size; i++) {
    printf("%d ", arr[i]);
  }
  printf("\n");

  my_free(arr);
  printf("\n");
}

void test_heap_dump(void) {
  printf("=== Test 10: Heap dump ===\n");

  int *a = (int *)my_malloc(64);
  int *b = (int *)my_malloc(128);
  int *c = (int *)my_malloc(256);

  my_free(b); // Create a hole

  my_heap_dump();

  my_free(a);
  my_free(c);
}

void test_edge_cases(void) {
  printf("=== Test 11: Edge cases ===\n");

  // malloc(0) should return NULL
  void *p1 = my_malloc(0);
  printf("malloc(0) = %p\n", p1);

  // free(NULL) should be safe
  my_free(NULL);
  printf("free(NULL) - no crash\n");

  // realloc(NULL, size) = malloc(size)
  int *p2 = (int *)my_realloc(NULL, 100);
  printf("realloc(NULL, 100) = %p\n", p2);
  my_free(p2);

  // realloc(ptr, 0) = free(ptr)
  int *p3 = (int *)my_malloc(100);
  void *p4 = my_realloc(p3, 0);
  printf("realloc(ptr, 0) = %p\n", p4);

  printf("\n");
}

void test_linked_list(void) {
  printf("=== Test 12: Linked list ===\n");

  typedef struct Node {
    int data;
    struct Node *next;
  } Node;

  Node *head = NULL;

  // Create list: 1 -> 2 -> 3 -> 4 -> 5
  for (int i = 5; i >= 1; i--) {
    Node *new_node = (Node *)my_malloc(sizeof(Node));
    new_node->data = i;
    new_node->next = head;
    head = new_node;
  }

  // Print list
  printf("List: ");
  Node *curr = head;
  while (curr) {
    printf("%d -> ", curr->data);
    curr = curr->next;
  }
  printf("NULL\n");

  // Free list
  while (head) {
    Node *temp = head;
    head = head->next;
    my_free(temp);
  }
  printf("List freed\n\n");
}

int main(void) {
  printf("\n");
  printf("╔═══════════════════════════════════════╗\n");
  printf("║   Custom Memory Allocator Demo        ║\n");
  printf("╚═══════════════════════════════════════╝\n\n");

  test_basic_malloc();
  test_calloc();
  test_realloc();
  test_string();
  test_struct();
  test_array_of_structs();
  test_2d_array();
  test_multiple_allocations();
  test_growing_array();
  test_heap_dump();
  test_edge_cases();
  test_linked_list();

  printf("All tests completed!\n\n");

  return 0;
}