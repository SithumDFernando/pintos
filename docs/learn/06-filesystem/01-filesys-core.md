---
module_id: "filesys-core"
title: "File System Core — filesys.c & filesys.h"
difficulty: "Moderate"
prerequisites:
  - "filesystem-overview"
  - "block-devices"
key_concepts:
  - "File system initialization"
  - "Formatting"
  - "Top-level file operations"
estimated_reading_time: "10 min"
source_files:
  - "pintos/src/filesys/filesys.c"
  - "pintos/src/filesys/filesys.h"
---

# File System Core — filesys.c & filesys.h

## 1. Executive Summary & Mental Model

`filesys.c` is the top-level entry point for all file system operations. It acts as a facade, combining the lower-level modules (`inode.c`, `directory.c`, `free-map.c`) into simple user-facing operations: create a file, open a file, delete a file.

**Real-world analogy**: If the file system is a library, `filesys.c` is the front desk. You walk up and say "I want to check out a book called 'hello.txt'", and the desk clerk (filesys) coordinates with the catalog (directory), the shelf manager (inode), and the inventory tracker (free-map) to get it for you.

## 2. Source Code Anatomy & Deep Dive

### Global State

```c
struct block *fs_device;   // The block device (disk partition) containing the file system
```

### `filesys_init(format)`
1. Finds the file system partition via `block_get_role(BLOCK_FILESYS)`.
2. Initializes the inode module and free map.
3. If `format` is true, calls `do_format()` to create a fresh file system.
4. Opens the free map from disk.

### `filesys_create(name, initial_size)`
Creates a new file in the root directory:
1. Opens the root directory.
2. Allocates a free sector for the new file's inode via `free_map_allocate()`.
3. Creates the inode on disk via `inode_create()`.
4. Adds the name-to-inode mapping in the directory via `dir_add()`.

### `filesys_open(name)`
Opens a file by name:
1. Opens the root directory.
2. Looks up the name in the directory to get the inode sector number.
3. Opens and returns a `struct file *` via `file_open()`.

### `filesys_remove(name)`
Deletes a file by name. Calls `dir_remove()` which marks the inode as "removed". The inode's sectors are not freed until the last opener closes the file.

### `do_format()`
Called once during `pintos -f`. Creates the free-map file and the root directory on a blank disk.

## 3. Lab Modifications Guide

This file is **not modified in any lab**. In **Project 2**, you call `filesys_open`, `filesys_create`, and `filesys_remove` from your system call handlers. You must protect these calls with a global lock because the file system is not thread-safe.
