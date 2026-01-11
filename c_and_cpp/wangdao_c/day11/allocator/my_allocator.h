#ifndef MY_ALLOCATOR_H
#define MY_ALLOCATOR_H

#include <stddef.h>

void *my_malloc(size_t size);
void *my_calloc(size_t num, size_t size);
void *my_realloc(void *ptr, size_t new_size);
void my_free(void *ptr);

// Debug functions
void my_heap_dump(void);
size_t my_heap_used(void);
size_t my_heap_free(void);

#endif