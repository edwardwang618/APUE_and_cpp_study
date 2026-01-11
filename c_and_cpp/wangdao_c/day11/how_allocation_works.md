## Memory Allocation: The Complete Picture

### The Big Picture First

When you write `int *p = malloc(100)`, you're asking for 100 bytes of memory. But where does this memory come from? The answer involves three layers: your program, the C library, and the operating system kernel.

```
    YOUR CODE
        │
        │  malloc(100)
        ▼
    ┌─────────────────────────────────────┐
    │         C LIBRARY (glibc)           │
    │                                     │
    │   - maintains free lists            │
    │   - tracks allocated blocks         │
    │   - splits and merges chunks        │
    │                                     │
    └─────────────────────────────────────┘
        │                           │
        │  sbrk()                   │  mmap()
        │  (small allocs)           │  (large allocs)
        ▼                           ▼
    ┌─────────────────────────────────────┐
    │         OPERATING SYSTEM            │
    │                                     │
    │   - manages virtual memory          │
    │   - maps virtual → physical pages   │
    │   - protects process memory         │
    │                                     │
    └─────────────────────────────────────┘
        │
        ▼
    ┌─────────────────────────────────────┐
    │          PHYSICAL RAM               │
    └─────────────────────────────────────┘
```

---

### Virtual Memory

**What it is:** Every process thinks it has its own private, contiguous memory starting from address 0. This is an illusion created by the CPU and OS working together.

**Why it exists:** Without virtual memory, programs would need to know exactly where in physical RAM they're loaded. Two programs couldn't both use address 0x1000. Programs could read each other's memory. It would be chaos.

**How it works:** The CPU has a component called the MMU (Memory Management Unit). Every time your program accesses memory, the MMU translates the virtual address to a physical address using page tables maintained by the OS.

```
    PROCESS A                    PHYSICAL RAM              PROCESS B
    (virtual)                                             (virtual)

    0x0000 ┌────────┐                                     ┌────────┐ 0x0000
           │        │           ┌────────────────┐        │        │
    0x1000 ├────────┤           │                │        ├────────┤ 0x1000
           │ code   │──────────►│  0x50000       │◄───────│ code   │
    0x2000 ├────────┤           ├────────────────┤        ├────────┤ 0x2000
           │ data   │──────────►│  0x73000       │        │ data   │────┐
           │        │           ├────────────────┤        │        │    │
           └────────┘           │  0x81000       │◄───────┴────────┘    │
                                ├────────────────┤                      │
                                │  0x92000       │◄─────────────────────┘
                                └────────────────┘

    Both processes use address 0x1000, but they map to different physical locations.
    The OS and MMU handle this transparently.
```

---

### Pages

**What they are:** Memory is divided into fixed-size chunks called pages, typically 4096 bytes (4KB) on most systems.

**Why this size:** It's a tradeoff. Smaller pages mean finer-grained control but more bookkeeping overhead. Larger pages waste more memory on partial fills. 4KB has been the sweet spot for decades, though modern systems also support "huge pages" (2MB or 1GB) for special cases.

**Page tables:** The OS maintains a data structure mapping each virtual page to a physical page (or marking it as not present). When you access unmapped memory, the CPU triggers a "page fault" and the OS decides what to do—maybe load data from disk, maybe kill your process with SIGSEGV.

```
    VIRTUAL PAGE        PAGE TABLE          PHYSICAL FRAME
    ┌──────────┐       ┌──────────────┐     ┌──────────┐
    │ Page 0   │──────►│ Frame 7      │────►│ Frame 7  │
    ├──────────┤       ├──────────────┤     ├──────────┤
    │ Page 1   │──────►│ Frame 2      │────►│ Frame 2  │
    ├──────────┤       ├──────────────┤     ├──────────┤
    │ Page 2   │──────►│ NOT PRESENT  │     │ Frame 3  │
    ├──────────┤       ├──────────────┤     ├──────────┤
    │ Page 3   │──────►│ Frame 9      │────►│ Frame 9  │
    └──────────┘       └──────────────┘     └──────────┘
                              │
                              ▼
                       Access Page 2?
                       PAGE FAULT! 
                       OS handles it.
```

---

### Process Address Space Layout

Every process has its memory organized into regions:

```
    HIGH ADDRESSES (0xFFFFFFFF on 32-bit)
    ┌─────────────────────────────────────┐
    │           KERNEL SPACE              │  You can't touch this.
    │      (mapped but protected)         │  System calls go here.
    ├─────────────────────────────────────┤ ◄── 0xC0000000 (typical)
    │                                     │
    │             STACK                   │  Local variables,
    │               │                     │  function arguments,
    │               ▼ grows down          │  return addresses.
    │                                     │
    │            · · · · ·                │  Unmapped gap
    │                                     │
    │               ▲ grows up            │
    │               │                     │
    │         MEMORY MAPPINGS             │  mmap() allocations,
    │         (mmap region)               │  shared libraries,
    │                                     │  large malloc chunks.
    │                                     │
    │            · · · · ·                │  Unmapped gap
    │                                     │
    │               ▲ grows up            │
    │               │                     │
    │             HEAP                    │  malloc() lives here
    │                                     │  (small allocations)
    ├─────────────────────────────────────┤ ◄── "program break" (brk)
    │             BSS                     │  Uninitialized globals
    │      (zero-initialized)             │  static int x;
    ├─────────────────────────────────────┤
    │            DATA                     │  Initialized globals
    │                                     │  static int x = 5;
    ├─────────────────────────────────────┤
    │            TEXT                     │  Your compiled code
    │        (read + execute)             │  (machine instructions)
    ├─────────────────────────────────────┤
    │         NULL GUARD                  │  Unmapped. Catches
    │                                     │  null pointer derefs.
    └─────────────────────────────────────┘
    LOW ADDRESSES (0x00000000)
```

---

### The Heap

**What it is:** A region of memory that grows upward from the end of your program's data segment. This is where most `malloc()` allocations come from.

**The program break (brk):** A pointer marking the current end of the heap. When the heap needs to grow, the program asks the OS to move this break higher.

```
    BEFORE: Heap is 8KB

              Data Segment
    ─────────────────────────
    ├─────────────────────────┤ ◄── original brk
    │                         │
    │     HEAP (8KB)          │
    │                         │
    ├─────────────────────────┤ ◄── current brk
              unmapped
    ─────────────────────────


    AFTER: sbrk(4096) - request 4KB more

              Data Segment
    ─────────────────────────
    ├─────────────────────────┤ ◄── original brk
    │                         │
    │     HEAP (8KB)          │
    │                         │
    ├─────────────────────────┤ ◄── old brk
    │                         │
    │   NEW SPACE (4KB)       │
    │                         │
    ├─────────────────────────┤ ◄── new brk
              unmapped
    ─────────────────────────
```

---

### sbrk() and brk()

**brk(addr):** Set the program break to a specific address. Returns 0 on success, -1 on failure.

**sbrk(increment):** Move the program break by `increment` bytes. Returns the *old* break address (so you know where your new memory starts). `sbrk(0)` returns the current break without changing it.

```c
void *current = sbrk(0);    // Where is the break now?
void *new_mem = sbrk(1000); // Give me 1000 more bytes
// new_mem points to the start of those 1000 bytes
```

**Limitation:** You can only grow or shrink from the end. You can't punch a hole in the middle and return it to the OS. This is why malloc needs to do its own bookkeeping—the OS doesn't track individual allocations, just the total heap size.

---

### mmap()

**What it is:** A system call that maps a region of virtual memory. More flexible than sbrk().

**Why malloc uses it:** For large allocations (typically 128KB+), malloc uses mmap() instead of sbrk(). The advantage is that mmap'd regions can be individually unmapped with munmap(), returning memory to the OS without affecting other allocations.

```c
// Allocate 1MB of anonymous memory
void *p = mmap(NULL,                    // let OS choose address
               1024*1024,               // size: 1MB
               PROT_READ | PROT_WRITE,  // readable and writable
               MAP_PRIVATE | MAP_ANONYMOUS,  // private, not file-backed
               -1,                      // no file descriptor
               0);                      // no offset

// Later, return it to the OS
munmap(p, 1024*1024);
```

**Anonymous mapping:** Memory not backed by any file. Just raw memory pages. That's what malloc uses.

```
    SMALL ALLOCATION (malloc uses sbrk)

    ┌──────────────────────────────────────────┐
    │                 HEAP                     │
    │   ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐       │
    │   │ 32B │ │ 64B │ │128B │ │ 32B │  ...  │
    │   └─────┘ └─────┘ └─────┘ └─────┘       │
    └──────────────────────────────────────────┘
         │
         └── all carved from contiguous heap


    LARGE ALLOCATION (malloc uses mmap)

    ┌──────────────────────────────────────────┐
    │           MMAP REGION                    │
    │                                          │
    │   ┌─────────────────────────────────┐    │
    │   │        2MB allocation           │    │
    │   │     (its own mmap region)       │    │
    │   └─────────────────────────────────┘    │
    │                                          │
    │   ┌───────────────────┐                  │
    │   │   512KB alloc     │                  │
    │   └───────────────────┘                  │
    │                                          │
    └──────────────────────────────────────────┘
         │
         └── each large alloc is independent,
             can be munmap'd separately
```

---

### Inside malloc(): Chunk Management

When you call malloc, the library doesn't just hand you raw memory from sbrk(). It maintains its own data structures to track what's allocated and what's free.

**Chunk structure:** Every allocation has a hidden header just before the pointer you receive:

```
    What malloc returns
           │
           ▼
    ┌──────────────┬─────────────────────────────────┐
    │   HEADER     │         YOUR DATA               │
    │   (hidden)   │                                 │
    ├──────────────┼─────────────────────────────────┤
    │              │                                 │
    │  prev_size   │  ← size of previous chunk       │
    │  size        │  ← size of this chunk + flags   │
    │              │                                 │
    │              │  your bytes start here ─────────┤
    │              │                                 │
    └──────────────┴─────────────────────────────────┘

    The size field's lowest bits are flags:
      bit 0: PREV_INUSE (previous chunk is allocated)
      bit 1: IS_MMAPPED (this chunk came from mmap)
      bit 2: NON_MAIN_ARENA (belongs to a thread arena)
```

**Free chunks** have additional data stored *in* the free space itself (since you're not using it):

```
    ┌──────────────────────────────────────────────┐
    │  prev_size                                   │
    │  size                                        │
    ├──────────────────────────────────────────────┤
    │  fd (forward pointer) ──────────► next free  │
    │  bk (backward pointer) ◄──────── prev free   │
    │                                              │
    │  (rest of free space, unused)                │
    │                                              │
    └──────────────────────────────────────────────┘
```

This is clever: free chunks form a doubly-linked list using space that would otherwise be wasted.

---

### Free Lists and Bins

Real malloc doesn't keep one big list of free chunks. That would be slow. Instead, it maintains multiple "bins" organized by size:

```
    BINS (simplified view of glibc malloc)

    Fast Bins (small, recently freed, LIFO)
    ┌─────────────────────────────────────────────────┐
    │  [16B] ──► chunk ──► chunk ──► NULL             │
    │  [24B] ──► chunk ──► NULL                       │
    │  [32B] ──► chunk ──► chunk ──► chunk ──► NULL   │
    │  ...                                            │
    │  [80B] ──► NULL                                 │
    └─────────────────────────────────────────────────┘

    Unsorted Bin (recently freed, not yet sorted)
    ┌─────────────────────────────────────────────────┐
    │  ──► chunk ──► chunk ──► chunk ──►              │
    └─────────────────────────────────────────────────┘

    Small Bins (exact sizes, < 512B)
    ┌─────────────────────────────────────────────────┐
    │  [16B]  ◄──► chunk ◄──► chunk ◄──►              │
    │  [24B]  ◄──► chunk ◄──►                         │
    │  ...                                            │
    │  [504B] ◄──►                                    │
    └─────────────────────────────────────────────────┘

    Large Bins (ranges of sizes, >= 512B)
    ┌─────────────────────────────────────────────────┐
    │  [512-576B]   ◄──► chunk ◄──►                   │
    │  [576-640B]   ◄──►                              │
    │  ...                                            │
    │  [1MB+]       ◄──► chunk ◄──►                   │
    └─────────────────────────────────────────────────┘
```

**Fast bins:** For very small allocations. Chunks aren't coalesced (merged), making alloc/free very fast. Single-linked list, LIFO (last in, first out).

**Unsorted bin:** When a chunk is freed, it often goes here first. On the next malloc, these get sorted into the proper bins.

**Small bins:** For exact small sizes. Double-linked list, FIFO (first in, first out). Chunks are coalesced.

**Large bins:** For larger allocations. Sorted by size within each bin so best-fit can be found quickly.

---

### Allocation Strategies

When malloc needs to find a chunk:

**First-fit:** Return the first chunk that's big enough. Simple but can cause fragmentation at the start of memory.

**Best-fit:** Find the smallest chunk that's big enough. Minimizes wasted space but takes longer to search.

**glibc's approach:** It's complicated. For small sizes, it checks fast bins first (exact match), then small bins. For larger sizes, it searches large bins for best fit. If nothing fits, it looks at the "top chunk" (the chunk at the end of the heap, adjacent to the break) and grows it with sbrk() if needed.

```
    malloc(100) - searching for a chunk:

    1. Check exact-size fast bin
       [104B] ──► NULL
       Nope, empty.

    2. Check exact-size small bin
       [104B] ◄──► chunk!
       Found one! Remove from list and return.

    If that failed:

    3. Search unsorted bin, sort chunks along the way
    
    4. Search larger small bins for something splittable
    
    5. Search large bins for best fit
    
    6. Use top chunk (extend heap if needed)
    
    7. Last resort: mmap a new region
```

---

### Splitting and Coalescing

**Splitting:** When you request 100 bytes but the only free chunk is 500 bytes, malloc splits it:

```
    BEFORE:
    ┌─────────────────────────────────────────────┐
    │  header  │         500 bytes FREE           │
    └─────────────────────────────────────────────┘

    AFTER malloc(100):
    ┌─────────────────────┬───────────────────────┐
    │ hdr │  104B USED    │ hdr │  ~380B FREE     │
    └─────────────────────┴───────────────────────┘
          │                     │
          │                     └── remainder goes back
          │                         to free list
          └── returned to you
              (104 = 100 + alignment)
```

**Coalescing:** When you free memory, malloc checks if neighboring chunks are also free. If so, it merges them into one bigger chunk:

```
    BEFORE free(B):
    ┌──────────────┬──────────────┬──────────────┐
    │   A: FREE    │   B: USED    │   C: FREE    │
    │    (100B)    │    (50B)     │    (200B)    │
    └──────────────┴──────────────┴──────────────┘

    AFTER free(B):
    ┌────────────────────────────────────────────┐
    │              MERGED: FREE                  │
    │               (~350B)                      │
    └────────────────────────────────────────────┘
```

This prevents fragmentation where you have many small free chunks but can't allocate a large block.

---

### free() Under the Hood

When you call `free(ptr)`:

```
    free(ptr)
        │
        ▼
    1. Check if ptr is NULL ──► if so, do nothing
        │
        ▼
    2. Back up to find chunk header
       (ptr - sizeof(header))
        │
        ▼
    3. Was this mmap'd? (check IS_MMAPPED flag)
       ├── YES ──► munmap() the region, done
       │
       └── NO ──► continue
        │
        ▼
    4. Coalesce with neighbors if they're free
        │
        ▼
    5. Add to appropriate bin
       - Small chunk? Fast bin or unsorted bin
       - Large chunk? Unsorted bin (sorted later)
        │
        ▼
    6. Should we return memory to OS?
       - If top chunk is huge, shrink with sbrk(-n)
       - Usually doesn't happen (costs performance)
```

**Important:** free() usually does NOT return memory to the OS. The freed memory stays in your process for future mallocs. This is why a process's memory usage often stays high even after freeing lots of memory.

---

### calloc() Under the Hood

```c
void *calloc(size_t count, size_t size)
```

calloc allocates memory for an array of `count` elements, each of `size` bytes, and **zeros all the bytes**.

```
    calloc(10, sizeof(int))  // 10 ints, 40 bytes total

        │
        ▼
    1. Check for overflow: count * size
       - If 10 * 4 would overflow size_t, return NULL
       - This is why calloc(huge, huge) safely fails
        │
        ▼
    2. Call malloc(count * size)
        │
        ▼
    3. Zero the memory with memset(ptr, 0, total_size)
        │
        ▼
    4. Return ptr
```

**Optimization:** For mmap'd memory, the kernel provides already-zeroed pages (for security—you shouldn't see another process's data). glibc can skip the memset in this case.

**Why use calloc over malloc + memset?**

The overflow check is a big one. Consider:
```c
size_t count = 1000000000;
size_t size = 1000;
void *p = malloc(count * size);  // OVERFLOW! Allocates tiny buffer
void *q = calloc(count, size);   // Returns NULL safely
```

---

### realloc() Under the Hood

```c
void *realloc(void *ptr, size_t new_size)
```

Changes the size of an existing allocation.

```
    realloc(ptr, new_size)
        │
        ▼
    1. ptr == NULL? ──► return malloc(new_size)
        │
        ▼
    2. new_size == 0? ──► free(ptr), return NULL
        │
        ▼
    3. Current chunk big enough?
       │
       ├── YES and much bigger ──► maybe split, return ptr
       │
       └── YES ──► return ptr (do nothing)
        │
        ▼
    4. Can we expand in place?
       - Is next chunk free AND big enough if merged?
       │
       ├── YES ──► merge chunks, return ptr
       │
       └── NO ──► continue
        │
        ▼
    5. Must relocate:
       a. malloc(new_size)
       b. memcpy(new_ptr, ptr, old_size)
       c. free(ptr)
       d. return new_ptr
```

**The memcpy is why realloc can be expensive.** If you're growing a 100MB buffer, and it can't expand in place, you're copying 100MB.

```
    SCENARIO A: Can expand in place (fast!)

    ┌───────────────┬───────────────────┐
    │  YOUR DATA    │   FREE CHUNK      │
    │   (100 KB)    │    (500 KB)       │
    └───────────────┴───────────────────┘
            │
            ▼ realloc to 400KB
    ┌───────────────────────────────────┬───────┐
    │          YOUR DATA                │ FREE  │
    │          (400 KB)                 │(200KB)│
    └───────────────────────────────────┴───────┘
    No copy needed! Just adjust headers.


    SCENARIO B: Must relocate (slow!)

    ┌───────────────┬───────────────────┐
    │  YOUR DATA    │  ANOTHER ALLOC    │
    │   (100 KB)    │    (in use)       │
    └───────────────┴───────────────────┘
            │
            ▼ realloc to 400KB

    1. Find space elsewhere:
    ┌───────────────┬──────────┬────────────────────────────┐
    │  YOUR DATA    │  IN USE  │      NEW LOCATION          │
    │   (100 KB)    │          │       (400 KB)             │
    └───────────────┴──────────┴────────────────────────────┘

    2. memcpy 100KB to new location
    3. free old location
    4. return new pointer (DIFFERENT from original!)
```

**Important:** Always use the return value of realloc:
```c
// WRONG - if realloc fails or moves, ptr is now invalid/leaked
ptr = realloc(ptr, new_size);  // if returns NULL, old ptr is lost!

// RIGHT
void *new_ptr = realloc(ptr, new_size);
if (new_ptr == NULL) {
    // handle error, ptr is still valid
} else {
    ptr = new_ptr;
}
```

---

### Thread Safety and Arenas

Modern programs are multithreaded. If every thread fought over one heap with one lock, performance would be terrible.

**Arenas:** glibc malloc maintains multiple independent heaps called arenas. Each arena has its own locks and free lists.

```
    Thread 1          Thread 2          Thread 3
        │                 │                 │
        ▼                 ▼                 ▼
    ┌────────┐       ┌────────┐       ┌────────┐
    │Arena 1 │       │Arena 2 │       │Arena 1 │
    │(main)  │       │        │       │(main)  │
    └────────┘       └────────┘       └────────┘
        │                 │                 │
        ▼                 ▼                 ▼
      malloc             malloc           malloc
    (no contention!)   (no contention!)  (might wait
                                          for thread 1)
```

The main arena uses sbrk(). Additional arenas use mmap(). When a thread first calls malloc, it tries to find an unlocked arena. If all are busy, it creates a new one (up to a limit).

---

### Summary

To tie it all together, here's what really happens when you write:

```c
int *arr = malloc(1000 * sizeof(int));
```

1. malloc receives request for 4000 bytes
2. Adds header overhead, aligns to 8 or 16 bytes → ~4016 bytes needed
3. Checks fast bins → nothing
4. Checks small bins → nothing this big
5. Checks unsorted bin → finds a 8000-byte chunk
6. Splits it: 4016 for you, ~3984 remains free
7. Returns pointer to your 4000 usable bytes
8. Your program uses it as an array of 1000 ints

When you free it:
1. free() backs up to find the header
2. Marks chunk as free
3. Checks neighbors—next chunk is free, merges them
4. Puts merged chunk in unsorted bin
5. Memory stays in your process for next malloc

The genius of malloc is that your program just sees simple memory allocation while all this complexity is hidden underneath.