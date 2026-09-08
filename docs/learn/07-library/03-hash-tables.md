---
module_id: "hash-tables"
title: "Hash Tables — hash.c"
difficulty: "Intermediate"
prerequisites:
  - "linked-lists"
key_concepts:
  - "Chained hash table"
  - "Intrusive data structure"
estimated_reading_time: "5 min"
source_files:
  - "pintos/src/lib/kernel/hash.c"
---

# Hash Tables — hash.c

## 1. Executive Summary & Mental Model

Pintos provides a built-in Hash Table implementation in `lib/kernel/hash.c`. Like the linked list, it is an **intrusive** data structure: your data structs must embed a `struct hash_elem`.

It uses separate chaining (an array of linked lists) to resolve collisions.

## 2. Usage & Deep Dive

To use the hash table, you must provide two function pointers:
1. **Hash Function**: Computes a bucket index for your element.
2. **Less Function**: Compares two elements to keep the linked list sorted (for faster searches).

**Example Usage**:
```c
struct page {
    void *vaddr;
    struct hash_elem elem;
};

// You write this
unsigned page_hash (const struct hash_elem *p_, void *aux) {
    const struct page *p = hash_entry (p_, struct page, elem);
    return hash_bytes (&p->vaddr, sizeof p->vaddr);
}

// You write this
bool page_less (const struct hash_elem *a_, const struct hash_elem *b_, void *aux) {
    const struct page *a = hash_entry (a_, struct page, elem);
    const struct page *b = hash_entry (b_, struct page, elem);
    return a->vaddr < b->vaddr;
}
```

## 3. Lab Modifications Guide

This file is **not modified in any lab**. 
However, it is extremely useful (and practically required) for **Project 3 (Virtual Memory)** to implement the Supplemental Page Table efficiently.
