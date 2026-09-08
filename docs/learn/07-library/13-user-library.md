---
module_id: "user-library"
title: "User-Space Library — lib/user/"
difficulty: "Moderate"
prerequisites:
  - "system-calls"
key_concepts:
  - "System call stubs"
  - "User program entry point"
estimated_reading_time: "5 min"
source_files:
  - "pintos/src/lib/user/syscall.c"
  - "pintos/src/lib/user/syscall.h"
  - "pintos/src/lib/user/entry.c"
  - "pintos/src/lib/user/console.c"
---

# User-Space Library — lib/user/

## 1. Executive Summary

The `lib/user/` directory contains code that is compiled **into user programs**, not into the kernel. It provides the C library interface that user programs use to invoke system calls.

## 2. Key Files

### `lib/user/syscall.c` — System Call Stubs

This file contains wrapper functions for every system call. For example:

```c
int write (int fd, const void *buffer, unsigned size) {
    return syscall3 (SYS_WRITE, fd, buffer, size);
}
```

The `syscall3` macro (defined in the same file) pushes three arguments and the syscall number onto the stack, then executes `int $0x30` to trap into the kernel. The return value comes back in `%eax`.

### `lib/user/entry.c` — User Program Entry Point

Every user program starts here, not at `main()`. The entry point `_start()`:
1. Calls `main(argc, argv)`.
2. Calls `exit(main's return value)`.

This ensures that even if a user program returns from `main` without calling `exit()`, the process still exits cleanly.

### `lib/user/console.c`

Provides `printf` for user programs by routing output through `SYS_WRITE` on fd 1 (stdout).

## 3. Lab Modifications Guide

These files are **not modified in any lab**. They are compiled into user test programs and provide the interface your kernel's `syscall_handler` must implement.
