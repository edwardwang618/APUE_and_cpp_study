#include "my_allocator.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// Block header structure
typedef struct block {
  size_t size;        // Size of user data (not including header)
  struct block *next; // Next block in list
  struct block *prev; // Previous block in list
  int free;           // 1 = free, 0 = allocated
  int magic;          // Magic number for validation
} block_t;

#define BLOCK_SIZE sizeof(block_t)
#define MAGIC 0x12345678
#define ALIGN(size) (((size) + 7) & ~7) // 8-byte alignment
#define MIN_BLOCK_SIZE 16

// Global state
static block_t *heap_start = NULL;
static block_t *heap_end = NULL;
static int initialized = 0;

// Initialize the allocator
static void allocator_init(void) {
  if (initialized)
    return;

  // Get initial heap (64 KB)
  size_t initial_size = 64 * 1024;

  heap_start = sbrk(initial_size);
  if (heap_start == (void *)-1) {
    heap_start = NULL;
    return;
  }

  // Set up first block (entire heap as one free block)
  heap_start->size = initial_size - BLOCK_SIZE;
  heap_start->next = NULL;
  heap_start->prev = NULL;
  heap_start->free = 1;
  heap_start->magic = MAGIC;

  heap_end = heap_start;
  initialized = 1;
}

// Extend heap when we run out of space
static block_t *extend_heap(size_t size) {
  size_t extend_size = size + BLOCK_SIZE;

  // Extend at least 4KB at a time
  if (extend_size < 4096) {
    extend_size = 4096;
  }

  block_t *new_block = sbrk(extend_size);
  if (new_block == (void *)-1) {
    return NULL;
  }

  new_block->size = extend_size - BLOCK_SIZE;
  new_block->next = NULL;
  new_block->prev = heap_end;
  new_block->free = 1;
  new_block->magic = MAGIC;

  if (heap_end) {
    heap_end->next = new_block;
  }
  heap_end = new_block;

  return new_block;
}

// Find a free block using first-fit strategy
static block_t *find_free_block(size_t size) {
  block_t *curr = heap_start;

  while (curr) {
    if (curr->free && curr->size >= size) {
      return curr;
    }
    curr = curr->next;
  }

  return NULL;
}

// Split a block if it's too big
static void split_block(block_t *block, size_t size) {
  size_t remaining = block->size - size - BLOCK_SIZE;

  // Only split if remaining is large enough
  if (remaining < MIN_BLOCK_SIZE) {
    return;
  }

  // Create new block in the remaining space
  block_t *new_block = (block_t *)((char *)(block + 1) + size);

  new_block->size = remaining;
  new_block->free = 1;
  new_block->magic = MAGIC;
  new_block->next = block->next;
  new_block->prev = block;

  if (block->next) {
    block->next->prev = new_block;
  } else {
    heap_end = new_block;
  }

  block->next = new_block;
  block->size = size;
}

// Coalesce adjacent free blocks
static void coalesce(block_t *block) {
  // Coalesce with next block
  if (block->next && block->next->free) {
    block->size += BLOCK_SIZE + block->next->size;
    block->next = block->next->next;

    if (block->next) {
      block->next->prev = block;
    } else {
      heap_end = block;
    }
  }

  // Coalesce with previous block
  if (block->prev && block->prev->free) {
    block->prev->size += BLOCK_SIZE + block->size;
    block->prev->next = block->next;

    if (block->next) {
      block->next->prev = block->prev;
    } else {
      heap_end = block->prev;
    }
  }
}

// Get block header from user pointer
static block_t *get_block(void *ptr) {
  block_t *block = (block_t *)ptr - 1;

  // Validate magic number
  if (block->magic != MAGIC) {
    fprintf(stderr, "ERROR: Invalid block or heap corruption!\n");
    return NULL;
  }

  return block;
}

// ============= Public Functions =============

void *my_malloc(size_t size) {
  if (size == 0) {
    return NULL;
  }

  // Initialize on first call
  if (!initialized) {
    allocator_init();
    if (!initialized) {
      return NULL;
    }
  }

  // Align size
  size = ALIGN(size);

  // Find free block
  block_t *block = find_free_block(size);

  // No free block found, extend heap
  if (!block) {
    block = extend_heap(size);
    if (!block) {
      return NULL;
    }
  }

  // Split block if too large
  split_block(block, size);

  // Mark as allocated
  block->free = 0;

  // Return pointer to user data (after header)
  return (void *)(block + 1);
}

void *my_calloc(size_t num, size_t size) {
  // Check for overflow
  size_t total = num * size;
  if (num != 0 && total / num != size) {
    return NULL; // Overflow
  }

  void *ptr = my_malloc(total);
  if (ptr) {
    memset(ptr, 0, total);
  }

  return ptr;
}

void *my_realloc(void *ptr, size_t new_size) {
  // realloc(NULL, size) = malloc(size)
  if (!ptr) {
    return my_malloc(new_size);
  }

  // realloc(ptr, 0) = free(ptr)
  if (new_size == 0) {
    my_free(ptr);
    return NULL;
  }

  block_t *block = get_block(ptr);
  if (!block) {
    return NULL;
  }

  new_size = ALIGN(new_size);

  // Current block is big enough
  if (block->size >= new_size) {
    // Could split here if much larger
    return ptr;
  }

  // Try to expand into next block if it's free
  if (block->next && block->next->free) {
    size_t combined = block->size + BLOCK_SIZE + block->next->size;

    if (combined >= new_size) {
      // Absorb next block
      block->size = combined;
      block->next = block->next->next;

      if (block->next) {
        block->next->prev = block;
      } else {
        heap_end = block;
      }

      // Split if necessary
      split_block(block, new_size);

      return ptr;
    }
  }

  // Must allocate new block and copy
  void *new_ptr = my_malloc(new_size);
  if (!new_ptr) {
    return NULL;
  }

  memcpy(new_ptr, ptr, block->size);
  my_free(ptr);

  return new_ptr;
}

void my_free(void *ptr) {
  if (!ptr) {
    return;
  }

  block_t *block = get_block(ptr);
  if (!block) {
    return;
  }

  // Check for double free
  if (block->free) {
    fprintf(stderr, "ERROR: Double free detected!\n");
    return;
  }

  // Mark as free
  block->free = 1;

  // Coalesce with adjacent free blocks
  coalesce(block);
}

// ============= Debug Functions =============

void my_heap_dump(void) {
  printf("\n========== HEAP DUMP ==========\n");

  if (!initialized) {
    printf("Heap not initialized\n");
    return;
  }

  block_t *curr = heap_start;
  int block_num = 0;

  while (curr) {
    printf("Block %d:\n", block_num);
    printf("  Address:  %p\n", (void *)curr);
    printf("  Size:     %zu bytes\n", curr->size);
    printf("  Status:   %s\n", curr->free ? "FREE" : "USED");
    printf("  Next:     %p\n", (void *)curr->next);
    printf("  Prev:     %p\n", (void *)curr->prev);
    printf("\n");

    curr = curr->next;
    block_num++;
  }

  printf("Total blocks: %d\n", block_num);
  printf("================================\n\n");
}

size_t my_heap_used(void) {
  size_t used = 0;
  block_t *curr = heap_start;

  while (curr) {
    if (!curr->free) {
      used += curr->size;
    }
    curr = curr->next;
  }

  return used;
}

size_t my_heap_free(void) {
  size_t free_space = 0;
  block_t *curr = heap_start;

  while (curr) {
    if (curr->free) {
      free_space += curr->size;
    }
    curr = curr->next;
  }

  return free_space;
}