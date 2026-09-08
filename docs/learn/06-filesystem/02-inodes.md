---
module_id: "inodes"
title: "Inodes — inode.c & inode.h"
difficulty: "Moderate"
prerequisites:
  - "filesys-core"
key_concepts:
  - "On-disk vs In-memory inodes"
  - "Contiguous allocation"
  - "Deny-write mechanism"
  - "Bounce buffers"
estimated_reading_time: "15 min"
source_files:
  - "pintos/src/filesys/inode.c"
  - "pintos/src/filesys/inode.h"
---

# Inodes — inode.c & inode.h

## 1. Executive Summary & Mental Model

An **inode** (Index Node) is the fundamental metadata object for a file. It records where the file's data lives on disk and how big the file is. Every file and directory in Pintos has exactly one inode.

**Real-world analogy**: The inode is a library card for a book. The card (inode) says "This book has 300 pages, and the pages are stored on shelves 5 through 10." The card is not the book itself, but without it you can't find the book.

## 2. Source Code Anatomy & Deep Dive

### On-Disk Inode (`struct inode_disk`)
```c
struct inode_disk {
    block_sector_t start;       /* First data sector. */
    off_t length;               /* File size in bytes. */
    unsigned magic;             /* Magic number (0x494e4f44). */
    uint32_t unused[125];       /* Padding to fill exactly 512 bytes. */
};
```
The base file system uses **contiguous allocation**: a file's data occupies consecutive sectors starting at `start`. This is simple but has a fatal limitation: files cannot grow, and fragmentation makes large files impossible.

### In-Memory Inode (`struct inode`)
```c
struct inode {
    struct list_elem elem;      /* Element in open_inodes list. */
    block_sector_t sector;      /* Sector number on disk. */
    int open_cnt;               /* Number of openers. */
    bool removed;               /* True if deleted. */
    int deny_write_cnt;         /* >0 means writes are denied. */
    struct inode_disk data;     /* Cached copy of on-disk data. */
};
```

### Key Operations

- **`inode_open(sector)`**: Checks the global `open_inodes` list first. If the inode is already open, it increments `open_cnt` and returns the same pointer (ensuring a single canonical in-memory copy).
- **`inode_read_at(inode, buffer, size, offset)`**: Reads data from the file. Uses `byte_to_sector()` to convert a file offset into a disk sector number, then calls `block_read()`. Uses a "bounce buffer" for partial-sector reads.
- **`inode_deny_write(inode)`**: Increments `deny_write_cnt`. While this counter is > 0, `inode_write_at` returns 0 (no bytes written). Used in P2 to prevent writing to running executables.
- **`inode_close(inode)`**: Decrements `open_cnt`. When it reaches 0, if the inode was marked `removed`, its disk sectors are freed via `free_map_release()`.

## 3. Lab Modifications Guide

This file is **not modified in P0-P2**. You use `file_deny_write()` (which calls `inode_deny_write()`) in P2 to protect running executables from modification.
