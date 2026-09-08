---
module_id: "directories"
title: "Directories — directory.c & directory.h"
difficulty: "Moderate"
prerequisites:
  - "inodes"
key_concepts:
  - "Directory entries"
  - "Name-to-inode mapping"
  - "Linear search"
estimated_reading_time: "8 min"
source_files:
  - "pintos/src/filesys/directory.c"
  - "pintos/src/filesys/directory.h"
---

# Directories — directory.c & directory.h

## 1. Executive Summary & Mental Model

A directory in Pintos is just a special file whose contents are an array of `struct dir_entry` records. Each entry maps a human-readable file name (like `"hello.txt"`) to the inode sector number where that file's metadata lives.

The base file system has only a **single, flat root directory** at sector 1. There are no subdirectories.

## 2. Source Code Anatomy & Deep Dive

```c
struct dir_entry {
    block_sector_t inode_sector;        /* Sector number of the file's inode. */
    char name[NAME_MAX + 1];           /* Null-terminated file name (max 14 chars). */
    bool in_use;                        /* Is this entry occupied? */
};
```

### Key Functions

- **`dir_lookup(dir, name, &inode)`**: Linear scan through entries looking for `name`. Returns the inode if found.
- **`dir_add(dir, name, inode_sector)`**: Finds an unused slot (or the end of the file) and writes a new entry.
- **`dir_remove(dir, name)`**: Finds the entry and sets `in_use = false`, then marks the inode as removed.
- **`dir_readdir(dir, name)`**: Iterates through entries one at a time. Used by `ls`-like operations.

## 3. Lab Modifications Guide

This file is **not modified in P0-P2**. You interact with it indirectly through `filesys_create`, `filesys_open`, and `filesys_remove` in your system call handlers.
