---
module_id: "page-directories"
title: "Page Directories — pagedir.c"
difficulty: "Intermediate"
prerequisites:
  - "virtual-addresses"
key_concepts:
  - "Hardware page tables"
  - "CR3 register"
  - "TLB invalidation"
estimated_reading_time: "15 min"
source_files:
  - "pintos/src/userprog/pagedir.c"
---

# Page Directories — pagedir.c

## 1. Executive Summary & Mental Model

While `vaddr.h` and `pte.h` provide the mathematical macros for paging, `pagedir.c` actually implements the high-level API for creating and managing a process's virtual address space. 

It acts as the Memory Management Unit (MMU) driver.

## 2. Source Code Anatomy & Deep Dive

A Page Directory (`pd`) is a 4 KB physical page containing 1024 Page Directory Entries (PDEs).

### `pagedir_create()`
Creates a new Page Directory for a new process. 
Crucially, it copies `init_page_dir` into the new directory. `init_page_dir` contains the mappings for the Kernel Space (3 GB to 4 GB). 
This ensures that **every user process shares the exact same kernel memory mappings**, but has a blank slate for its own User Space (0 to 3 GB).

### `pagedir_activate()`
Called during a context switch. It loads the physical address of the page directory into the CPU's `CR3` register. As soon as `CR3` is updated, the CPU instantly starts translating addresses using the new directory.

### `pagedir_set_page(pd, upage, kpage, writable)`
The most important function for `load()`. 
It maps a user virtual address (`upage`) to a physical address (represented here by its kernel virtual alias `kpage`).
1. Uses `lookup_page()` to find the correct Page Table Entry (PTE). If the intermediate Page Table doesn't exist, it allocates one.
2. Updates the PTE with the physical address and flags (User accessible, Present, Writable).

### The Translation Lookaside Buffer (TLB)
The CPU caches recent page translations in a hardware cache called the TLB. 
If `pagedir_clear_page()` unmaps a page, the CPU might still use the cached translation! 
Therefore, modifying an active page directory requires calling `invalidate_pagedir()`, which reloads `CR3` to flush the TLB cache.

## 3. Lab Modifications Guide

This file is generally **not modified in Project 2**. 
However, in **Project 3 (Virtual Memory)**, you will heavily augment or replace this file. You will need to track Supplemental Page Tables, handle swapping pages to disk, and manage the "Accessed" (`PTE_A`) and "Dirty" (`PTE_D`) bits to implement a page replacement algorithm (like LRU clock).
