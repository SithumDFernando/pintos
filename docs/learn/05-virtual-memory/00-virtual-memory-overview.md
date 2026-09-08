---
module_id: "virtual-memory-overview"
title: "Virtual Memory — vm/ (Project 3)"
difficulty: "Advanced"
prerequisites:
  - "page-directories"
  - "memory-allocation"
key_concepts:
  - "Supplemental Page Table"
  - "Frame Table"
  - "Swap Table"
  - "Lazy Loading"
estimated_reading_time: "5 min"
source_files:
  - "pintos/src/vm/"
---

# Virtual Memory — vm/ (Project 3)

## 1. Executive Summary & Mental Model

The `vm/` directory is initially almost empty. In **Project 3**, you must build a complete Virtual Memory subsystem here.

Currently, Pintos maps virtual pages to physical frames 1-to-1 immediately during `load()`. If the user program is 10 MB, Pintos needs 10 MB of physical RAM.
Virtual Memory changes this. It provides the illusion of infinite memory by moving inactive pages to the hard disk (the "swap slot") and only keeping active pages in physical RAM.

## 2. Key Components to Build

If you were to complete Project 3, you would design three main data structures:

1. **Supplemental Page Table (SPT)**:
   The hardware page table (`pagedir.c`) only knows if a page is present or not. The SPT tracks *where* the data actually is: in a file, in swap space, or just all zeros. It enables **Lazy Loading** (don't load a page from disk until the program actually accesses it and causes a Page Fault).

2. **Frame Table**:
   Tracks every physical page (frame) in the system and which user process currently owns it. If memory is full, the OS uses a page replacement algorithm (like Clock/LRU) to pick a frame to evict.

3. **Swap Table**:
   Tracks which slots on the swap disk are free or in use. When a page is evicted from the Frame Table, it is written here.

## 3. Lab Modifications Guide

Since your module only requires completing up to **Lab 2**, you will not need to implement these files.
