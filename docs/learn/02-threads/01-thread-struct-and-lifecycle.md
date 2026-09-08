---
module_id: "thread-struct-and-lifecycle"
title: "Thread Structure & Lifecycle — thread.h/c (Full Depth)"
difficulty: "Advanced"
prerequisites:
  - "threading-overview"
key_concepts:
  - "struct thread memory layout"
  - "Thread creation and execution"
  - "Priority scheduling"
  - "MLFQS scheduling calculations"
estimated_reading_time: "30 min"
source_files:
  - "pintos/src/threads/thread.h"
  - "pintos/src/threads/thread.c"
---

# Thread Structure & Lifecycle — thread.h/c ⭐ P1+P2

## 1. Executive Summary & Mental Model

`thread.c` and `thread.h` are the foundational files for multitasking in Pintos. They define what a thread is (`struct thread`), how it is created (`thread_create`), how it runs, and how it dies. 

**Real-world analogy**: `struct thread` is the **employee dossier** for a chef in our restaurant. It contains their ID, their name, their current status (working, waiting, off-shift), and their priority (head chef vs prep cook). The functions in `thread.c` are the HR procedures for hiring, firing, and scheduling shifts.

**Primary CS problems solved**:
- State management for execution contexts.
- Context switch coordination.
- Dispatching and scheduling policies.

## 2. Theoretical Foundations

### The 4 KB Page Layout

Every thread in Pintos gets exactly one 4 KB page of physical memory. This single page holds **both** the thread's metadata (`struct thread`) and its kernel execution stack.

Because they share the same fixed-size space, they grow towards each other. If the stack grows too deep, it will overwrite the metadata, causing a silent and catastrophic failure (stack overflow).

### Priority Scheduling

In a strict priority scheduler, the CPU is always given to the highest-priority thread in the `THREAD_READY` state. 
- If a thread with priority 60 becomes ready while a thread with priority 50 is running, the running thread must yield **immediately**.
- Round-robin is only used as a tie-breaker for threads with the same priority.

## 3. Architecture & Visual Topography

### Memory Layout of a Thread Page

```
    4 kB +---------------------------------+
         |          kernel stack           |
         |                |                |
         |                V                |
         |         grows downward          |
         |                                 |
         |                                 |
         |                                 |
         +---------------------------------+
         |              magic              |  ← offset ~ 32 bytes
         |             pagedir             |  ← (USERPROG only)
         |              elem               |
         |             allelem             |
         |            priority             |
         |              stack              |  ← Saved %esp during switch
         |              name               |
         |             status              |
         |               tid               |
    0 kB +---------------------------------+  ← struct thread starts here
```

To find the `struct thread` pointer for the currently executing code, Pintos takes the current stack pointer (`%esp`) and masks off the lower 12 bits (rounds down to the nearest 4 KB boundary). See `running_thread()`.

## 4. Source Code Anatomy & Deep Dive

### `thread.h` — `struct thread`

```c
struct thread
  {
    tid_t tid;                          /* Thread identifier. */
    enum thread_status status;          /* Thread state. */
    char name[16];                      /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */
    int priority;                       /* Priority. */
    struct list_elem allelem;           /* List element for all threads list. */
    struct list_elem elem;              /* List element for ready_list / wait lists */

#ifdef USERPROG
    uint32_t *pagedir;                  /* Page directory. */
#endif

    unsigned magic;                     /* Detects stack overflow (must be THREAD_MAGIC). */
  };
```

**Invariant**: `magic` must always equal `0xcd6abf4b`. It is checked in `thread_current()`.

### `thread.c` — Key Functions

#### `thread_create()`
Allocates a page, initializes `struct thread` at the bottom, sets up the initial stack frame at the top (faking a context switch frame so `switch_threads` can jump to it), and adds it to the `ready_list`.

#### `thread_tick()`
Called 100 times per second by the timer interrupt. Increments `thread_ticks`. If `thread_ticks >= TIME_SLICE` (4), it forces a preemption by calling `intr_yield_on_return()`.

#### `next_thread_to_run()`
Pops the first thread off `ready_list`. If the list is empty, returns the `idle_thread`.

#### `schedule()`
The heart of context switching. 
1. Gets the current thread and the next thread.
2. Calls `switch_threads(cur, next)` (written in assembly).
3. Calls `thread_schedule_tail(prev)` to clean up if the previous thread died.

## 5. Runtime Execution & Sequence

### The Birth of a Thread

```mermaid
sequenceDiagram
    participant Caller
    participant CREATE as thread_create()
    participant ALLOC as palloc_get_page()
    participant SCHED as ready_list

    Caller->>CREATE: thread_create("my_thread", 31, func)
    CREATE->>ALLOC: allocate 4KB page
    ALLOC-->>CREATE: page pointer
    CREATE->>CREATE: init_thread() (set name, tid, magic)
    CREATE->>CREATE: alloc_frame() for switch_threads
    CREATE->>CREATE: alloc_frame() for switch_entry
    CREATE->>CREATE: alloc_frame() for kernel_thread
    CREATE->>SCHED: list_push_back(&ready_list)
    CREATE-->>Caller: return new tid
```

## 6. Edge Cases, Failure Modes & Resilience

| Failure | Symptom | Defense |
|---|---|---|
| Stack Overflow | Assertion failure in `thread_current()` | The `magic` field is placed at the end of the struct. As the stack grows down, it will overwrite `magic` first. |
| Accessing freed thread | Page fault or corrupted data | Threads are only freed in `thread_schedule_tail()` *after* the context switch is entirely complete. |

## 7. Hands-On Verification & Challenges

<details>
<summary><strong>Q1: Why does <code>thread_create</code> fake three stack frames?</strong></summary>

When the scheduler eventually switches to this new thread, it jumps into `switch_threads()`, expecting to pop registers and return to `switch_entry()`. `switch_entry` expects to return to `kernel_thread()`, which then calls the actual provided function. Faking the stack sets up this return chain so the new thread starts executing naturally.
</details>

## 8. Lab Modifications Guide

### ⭐ P1: Priority Scheduling

**Task 1: Priority-based ready queue**
By default, `ready_list` is a FIFO queue. For P1, the highest priority thread must run first.

*   **Option A (Insert Sorted - Recommended)**: Change `list_push_back(&ready_list, ...)` in `thread_unblock`, `thread_yield`, and `init_thread` to `list_insert_ordered()`.
    *   *Trade-offs*: O(N) insertion, O(1) scheduling. Much simpler to implement using Pintos list functions.
*   **Option B (Search on Schedule)**: Keep FIFO insertion, but modify `next_thread_to_run()` to scan the list for the highest priority using `list_max()`.
    *   *Trade-offs*: O(1) insertion, O(N) scheduling.

**Task 2: Immediate Preemption**
When a new thread is created or unblocked with a higher priority than the current running thread, the current thread must yield.
*   **Action**: Modify `thread_create()` and `thread_set_priority()` to check if the new/updated priority is strictly greater than `thread_current()->priority`. If so, call `thread_yield()`.

### ⭐ P1: Priority Donation (Infrastructure)

To support priority donation (implemented mainly in `synch.c`), `struct thread` needs new fields:

*   **Option A (Single Donated Priority)**:
    ```c
    int base_priority;      // The original priority
    struct lock *wait_on_lock; // The lock this thread is waiting for
    struct list donations;  // List of threads donating to us
    ```
    *   *Trade-offs*: Handles multiple donations well. You need a way to track the maximum donated priority efficiently.

### ⭐ P1: Advanced Scheduler (MLFQS)

When run with `-mlfqs`, priority is ignored and calculated dynamically.

**Task 1: Add Fields to `struct thread`**
```c
int nice;
fixed_point_t recent_cpu;
```

**Task 2: Fixed-Point Arithmetic**
Pintos doesn't support floats. Create `threads/fixed-point.h` with macros for 17.14 fixed-point math:
```c
#define F (1 << 14)
#define INT_TO_FP(n) ((n) * F)
#define FP_ADD(x, y) ((x) + (y))
#define FP_MULT(x, y) (((int64_t)(x)) * (y) / F)
// ... etc
```

**Task 3: Update Calculations in `thread_tick()`**
*   Every tick: `recent_cpu` of running thread ++
*   Every second (100 ticks):
    *   Update `load_avg`
    *   Update `recent_cpu` for **all** threads (use `thread_foreach` or iterate `all_list`)
*   Every 4th tick:
    *   Recalculate `priority` for **all** threads.
    *   Yield if the running thread is no longer the highest priority.

### ⭐ P2: Process Management Bookkeeping

In Project 2, a thread represents a user process. You must add fields to track its life:

```c
#ifdef USERPROG
  int exit_status;
  struct list children;      // List of child processes
  struct semaphore wait_sema; // For parent to wait on
  struct file *executable;   // To deny writes to the running executable
  // ... file descriptor table
#endif
```
