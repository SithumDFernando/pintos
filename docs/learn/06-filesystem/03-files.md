---
module_id: "files"
title: "Files — file.c & file.h"
difficulty: "Moderate"
prerequisites:
  - "inodes"
key_concepts:
  - "File position tracking"
  - "Read/Write/Seek/Tell"
estimated_reading_time: "8 min"
source_files:
  - "pintos/src/filesys/file.c"
  - "pintos/src/filesys/file.h"
---

# Files — file.c & file.h

## 1. Executive Summary & Mental Model

A `struct file` represents an **open file**. It wraps an inode and adds a current read/write position (`pos`), like a bookmark in a book. Multiple threads can open the same file independently, each with their own position.

## 2. Source Code Anatomy & Deep Dive

```c
struct file {
    struct inode *inode;    /* File's inode. */
    off_t pos;              /* Current position (the "bookmark"). */
    bool deny_write;        /* Has file_deny_write() been called? */
};
```

### Key Functions

| Function | Purpose |
|---|---|
| `file_open(inode)` | Creates a new `struct file` for the given inode. |
| `file_read(file, buf, size)` | Reads `size` bytes starting at `file->pos`, then advances `pos`. |
| `file_write(file, buf, size)` | Writes `size` bytes starting at `file->pos`, then advances `pos`. |
| `file_seek(file, new_pos)` | Sets the position to `new_pos`. |
| `file_tell(file)` | Returns the current position. |
| `file_length(file)` | Returns the file's total size in bytes. |
| `file_deny_write(file)` | Prevents any write operations on the underlying inode. |
| `file_close(file)` | Re-enables writes if denied, closes the inode, frees the struct. |

## 3. Lab Modifications Guide

This file is **not modified in any lab**. In P2, your `syscall_handler` will call these functions (`file_read`, `file_write`, `file_seek`, `file_tell`, `file_length`, `file_close`) to implement the file-related system calls. You will also call `file_deny_write` in `load()` to protect running executables.
