---
module_id: "memory-allocation"
title: "Memory Allocation — palloc.c & malloc.c"
difficulty: "Intermediate"
prerequisites:
  - "boot-init"
key_concepts:
  - "Page allocation vs Block allocation"
  - "User pool vs Kernel pool"
  - "Memory bitmaps"
  - "Slab allocator (malloc implementation)"
estimated_reading_time: "15 min"
source_files:
  - "pintos/src/threads/palloc.h"
  - "pintos/src/threads/palloc.c"
  - "pintos/src/threads/malloc.h"
  - "pintos/src/threads/malloc.c"
---

# Memory Allocation — palloc.c & malloc.c

## 1. Executive Summary & Mental Model

Pintos has two levels of memory allocation:
1. **The Page Allocator (`palloc.c`)**: Hands out memory in large, 4 KB chunks (pages). It's the foundation of memory management in Pintos.
2. **The Block Allocator (`malloc.c`)**: Built *on top of* the page allocator. It hands out smaller, arbitrary-sized chunks of memory (like standard C `malloc`).

**Real-world analogy**: 
Imagine you are building a city.
- `palloc` is the government selling **entire city blocks** (acres of land) to developers. You can't buy just a tiny sliver of land from `palloc`; you must buy a full block.
- `malloc` is a developer who buys a city block from the government, subdivides it into **individual apartments**, and rents them out to people who only need a small space.

**Primary CS problems solved**:
- Managing free vs used memory.
- Preventing memory fragmentation.
- Separating kernel memory from user memory to prevent resource starvation.

## 2. Theoretical Foundations

### The Two Pools

`palloc` divides the total physical memory (up to 64 MB) into two separate pools:
1. **Kernel Pool**: Used for kernel data structures (threads, page tables, `malloc` arenas).
2. **User Pool**: Used exclusively for user process memory (code, data, and stack pages).

**Why separate them?** 
If user programs go wild and allocate all available memory, the kernel still needs memory to operate (e.g., to create a thread to kill the rogue process, or to run the file system). By reserving half the memory for the kernel, Pintos ensures the kernel never starves due to user actions.

### Bitmaps

To keep track of which pages are free and which are used, `palloc` uses a **bitmap**. 
- 1 bit = 1 page (4 KB).
- If bit `N` is 0, page `N` is free.
- If bit `N` is 1, page `N` is used.

### The Slab Allocator Pattern (`malloc`)

Pintos' `malloc` implementation is a simplified slab allocator. It only allocates blocks in powers of 2 (16 bytes, 32 bytes, ..., up to 1024 bytes). 
If you request 20 bytes, `malloc` rounds it up and gives you a 32-byte block. This causes some internal fragmentation (wasted space inside the block) but completely eliminates external fragmentation and makes allocation extremely fast.

## 3. Architecture & Visual Topography

### Memory Map of Physical RAM

```
0x00000000  +-----------------------+
            | Bootloader / BIOS     |
0x00020000  +-----------------------+
            | Kernel Code & Data    |
            | (up to 512 KB)        |
0x00100000  +-----------------------+ <-- 1 MB boundary (ptov(1024*1024))
            | Kernel Pool Bitmap    |
            | Kernel Free Pages     |
            |                       |
            |   (approx 50%)        |
            |                       |
            +-----------------------+ <-- user_pool.base
            | User Pool Bitmap      |
            | User Free Pages       |
            |                       |
            |   (approx 50%)        |
            |                       |
0x04000000  +-----------------------+ <-- End of RAM (up to 64 MB)
```

### `malloc` Arena Layout

When `malloc` needs more 32-byte blocks, it calls `palloc_get_page()` to get a 4 KB arena, and formats it like this:

```
    4 kB +-------------------------+
         |       Block 127         | (32 bytes)
         +-------------------------+
         |          ...            |
         +-------------------------+
         |       Block 1           | (32 bytes)
         +-------------------------+
         |       Block 0           | (32 bytes)
         +-------------------------+
         | struct arena (Header)   | (16 bytes: magic, desc, free_cnt)
    0 kB +-------------------------+ 
```

## 4. Source Code Anatomy & Deep Dive

### `palloc.c`

```c
enum palloc_flags
  {
    PAL_ASSERT = 001,           /* Panic on failure. */
    PAL_ZERO = 002,             /* Zero page contents. */
    PAL_USER = 004              /* User page. */
  };
```
These flags are bitmasks. You can combine them: `palloc_get_page(PAL_USER | PAL_ZERO)`.

#### `palloc_get_multiple()`
1. Selects the pool based on `PAL_USER` flag.
2. Acquires the pool's lock.
3. Calls `bitmap_scan_and_flip()` to find `page_cnt` consecutive 0 bits and set them to 1.
4. Releases the lock.
5. Returns the memory address, optionally zeroing it if `PAL_ZERO` is set. If no memory is found, panics if `PAL_ASSERT` is set, otherwise returns `NULL`.

#### `palloc_free_multiple()`
1. Checks which pool the pointer belongs to (using memory bounds).
2. Calculates the bit index: `page_idx = pg_no(pages) - pg_no(pool->base)`.
3. Calls `bitmap_set_multiple()` to flip those bits back to 0.

### `malloc.c`

```c
struct desc
  {
    size_t block_size;          /* Size of each element in bytes. (e.g., 32) */
    size_t blocks_per_arena;    /* Number of blocks in an arena. */
    struct list free_list;      /* List of free blocks across all arenas. */
    struct lock lock;           /* Mutual exclusion for this descriptor. */
  };
```
There is an array `static struct desc descs[10];` initialized in `malloc_init()` to handle sizes 16, 32, 64, 128, 256, 512, and 1024.

#### `malloc(size)`
1. Finds the smallest descriptor `d` where `d->block_size >= size`.
2. If `size > 1024`, it skips descriptors entirely and calls `palloc_get_multiple()` directly (creating a "big block").
3. If `d->free_list` is empty, calls `palloc_get_page()` to get a new arena, divides it into blocks, and adds them to `free_list`.
4. Pops a block off `free_list` and returns it.

#### Finding the Arena Header
How does `free(void *p)` know which descriptor a block belongs to?
Since every arena is exactly 1 page (4 KB) and is page-aligned, the header (`struct arena`) is always at the *start* of the page.
```c
static struct arena *block_to_arena (struct block *b) {
  // Round the pointer down to the nearest 4KB boundary!
  struct arena *a = pg_round_down (b);
  return a;
}
```

## 5. Runtime Execution & Sequence

Let's trace a call to `malloc(20)`.

1. `malloc(20)` searches descriptors. It finds the 32-byte descriptor.
2. The 32-byte descriptor's `free_list` is empty.
3. `malloc` calls `palloc_get_page(0)` (from the kernel pool).
4. `palloc` scans the kernel bitmap, finds page #42, marks it used, and returns `0xC002A000`.
5. `malloc` places `struct arena` at `0xC002A000`.
6. It slices the rest of the page into 127 blocks of 32 bytes and adds them to `free_list`.
7. It removes the first block from `free_list` and returns it to the caller (`0xC002A010`).
8. The caller now has 32 bytes of memory, though they only asked for 20.

## 6. Edge Cases, Failure Modes & Resilience

| Issue | Pintos Behavior |
|---|---|
| User asks for memory but User Pool is full | `palloc` returns NULL. (User processes will page fault and be killed). |
| Out of bounds write in a `malloc` block | Corrupts adjacent blocks. If it hits the start of the page, it corrupts the `ARENA_MAGIC` number, causing a panic during `free()`. |
| Double free | `free()` adds the block back to the `free_list` twice. Next time it's allocated, it will be handed out to two different callers simultaneously (disaster). Pintos `#ifndef NDEBUG` fills freed memory with `0xCC` to help detect use-after-free bugs. |

## 7. Hands-On Verification & Challenges

<details>
<summary><strong>Q1: Why does <code>palloc_free_page</code> not need to be told whether the page came from the user pool or kernel pool?</strong></summary>

Because all physical memory is neatly divided in half. `palloc_free_multiple` compares the pointer against the boundaries of the `kernel_pool` and `user_pool` to figure out which one it belongs to automatically.
</details>

<details>
<summary><strong>Q2: If a thread calls <code>malloc(5000)</code>, where does the memory come from?</strong></summary>

The request is larger than 1024 bytes (the max block size). `malloc` handles this as a "big block". It calculates `page_cnt = DIV_ROUND_UP(5000 + 16, 4096) = 2`. It calls `palloc_get_multiple(0, 2)` to get 2 contiguous pages from the kernel pool.
</details>

## 8. Lab Modifications Guide

These files are generally **not modified in any lab**. 
However, you will use them extensively:
- Use `palloc_get_page(PAL_USER | PAL_ZERO)` in **Project 2/3** when setting up memory for a user process.
- Use `malloc()` for dynamic data structures in the kernel (like lists of open files or child processes).
- **CRITICAL WARNING**: Do not use large arrays as local variables (e.g., `char buffer[4096]`). Thread stacks are limited to 4 KB. Local variables go on the stack. You will overflow the stack and crash the kernel. Always use `malloc` or `palloc` for anything larger than a hundred bytes.
