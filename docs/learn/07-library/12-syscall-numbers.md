---
module_id: "syscall-numbers"
title: "System Call Numbers — syscall-nr.h"
difficulty: "Moderate"
prerequisites:
  - "system-calls"
key_concepts:
  - "System call numbering"
estimated_reading_time: "3 min"
source_files:
  - "pintos/src/lib/syscall-nr.h"
---

# System Call Numbers — syscall-nr.h

## 1. Executive Summary

This header defines the numeric constants for each system call. When a user program calls `write(fd, buf, size)`, the user-space library pushes `SYS_WRITE` (9) onto the stack before executing `int 0x30`.

## 2. System Call Table

```c
enum {
    SYS_HALT,       /* 0  - Terminate Pintos. */
    SYS_EXIT,       /* 1  - Terminate this process. */
    SYS_EXEC,       /* 2  - Start another process. */
    SYS_WAIT,       /* 3  - Wait for a child process to die. */
    SYS_CREATE,     /* 4  - Create a file. */
    SYS_REMOVE,     /* 5  - Delete a file. */
    SYS_OPEN,       /* 6  - Open a file. */
    SYS_FILESIZE,   /* 7  - Obtain a file's size. */
    SYS_READ,       /* 8  - Read from a file. */
    SYS_WRITE,      /* 9  - Write to a file. */
    SYS_SEEK,       /* 10 - Change position in a file. */
    SYS_TELL,       /* 11 - Report current position in a file. */
    SYS_CLOSE,      /* 12 - Close a file. */
    /* Project 3 and optionally project 4. */
    SYS_MMAP,       /* 13 - Map a file into memory. */
    SYS_MUNMAP,     /* 14 - Remove a memory mapping. */
    /* Project 4 only. */
    SYS_CHDIR,      /* 15 */
    SYS_MKDIR,      /* 16 */
    SYS_READDIR,    /* 17 */
    SYS_ISDIR,      /* 18 */
    SYS_INUMBER     /* 19 */
};
```

In your P2 `syscall_handler`, you read this number from `*(int *)(f->esp)` and use a `switch` statement to dispatch to the correct handler.

## 3. Lab Modifications Guide

This file is **not modified in any lab**. You just read these constants.
