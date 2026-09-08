---
module_id: "linked-lists"
title: "Linked Lists — list.c & list.h (Full Depth)"
difficulty: "Intermediate"
prerequisites:
  - "utilities-overview"
key_concepts:
  - "Intrusive doubly-linked lists"
  - "list_entry macro"
  - "O(1) insertion/deletion"
estimated_reading_time: "15 min"
source_files:
  - "pintos/src/lib/kernel/list.h"
  - "pintos/src/lib/kernel/list.c"
---

# Linked Lists — list.c & list.h ⭐ P1-P4

## 1. Executive Summary & Mental Model

Pintos uses a specialized type of linked list called an **intrusive doubly-linked list**. 

In a standard linked list (like Java's `LinkedList`), the list "node" object contains a pointer to your data. 
In an *intrusive* list, **your data object contains the list node**. 

**Real-world analogy**: 
- **Standard List**: A train where every passenger is put inside a separate shipping crate, and the crates are hooked together.
- **Intrusive List**: Every passenger wears a belt with two hooks (front and back). The passengers hook directly to each other.

This design avoids dynamic memory allocation (`malloc`) when adding items to the list, which is crucial for an operating system kernel.

## 2. Source Code Anatomy & Deep Dive

### The `list_elem`

```c
struct list_elem {
  struct list_elem *prev;     /* Previous list element. */
  struct list_elem *next;     /* Next list element. */
};
```
This is the "belt with two hooks". You embed this inside your own structs:

```c
struct thread {
  tid_t tid;
  // ...
  struct list_elem elem;      // <--- The hook!
};
```

### The `list`

```c
struct list {
  struct list_elem head;      /* List head. */
  struct list_elem tail;      /* List tail. */
};
```
The list itself contains two dummy elements (sentinels). 
An empty list looks like this: `head.next == &tail` and `tail.prev == &head`. 
Sentinels eliminate edge cases (like inserting into an empty list or deleting the last item) because every real element always has a valid `prev` and `next`.

### The Magic Macro: `list_entry`

If the list only links `struct list_elem`s together, how do you get your `struct thread` back?

```c
#define list_entry(LIST_ELEM, STRUCT, MEMBER) \
        ((STRUCT *) ((uint8_t *) &(LIST_ELEM)->next \
                     - offsetof (STRUCT, MEMBER.next)))
```

This macro uses pointer arithmetic. It says: "I know the `list_elem` is located exactly 20 bytes from the start of `struct thread`. So, take the memory address of the `list_elem`, subtract 20 bytes, and cast it to a `struct thread *`."

**Usage**:
```c
struct list_elem *e = list_pop_front(&ready_list);
struct thread *t = list_entry(e, struct thread, elem);
```

## 3. Important Functions

- **`list_init(list)`**: Initializes the head and tail sentinels.
- **`list_push_back(list, elem)`**: Adds to the end. O(1).
- **`list_push_front(list, elem)`**: Adds to the beginning. O(1).
- **`list_pop_front(list)`**: Removes and returns the first element. O(1).
- **`list_remove(elem)`**: Removes an element from wherever it is in the list. O(1). (This is why it's a doubly-linked list!)
- **`list_insert_ordered(list, elem, less_func, aux)`**: Inserts an element in sorted order. O(N).

### Iterating over a list
Do not use standard `for(int i=0; i<N; i++)` loops. Use the list pointers:
```c
for (struct list_elem *e = list_begin (&my_list); 
     e != list_end (&my_list); 
     e = list_next (e)) 
  {
    struct thread *t = list_entry (e, struct thread, elem);
    // Do something with t...
  }
```

## 4. Lab Modifications Guide

This file is **not modified in any lab**.
However, you will use it constantly:
- **P1**: Modifying `ready_list` insertions to use `list_insert_ordered` for Priority Scheduling.
- **P1**: Managing priority donation lists.
- **P2**: Managing lists of child processes or open file descriptors in `struct thread`.

### Common Bug: The "Element Already in List" Panic
A `list_elem` can only be in **one list at a time**. If you try to push an element that is already in a list, Pintos will panic. 
- *Fix*: If a thread needs to be in multiple lists (e.g., the `all_list` and a `waiters` list), you must embed **two separate `list_elem`s** inside `struct thread` (e.g., `struct list_elem allelem;` and `struct list_elem waitelem;`).
