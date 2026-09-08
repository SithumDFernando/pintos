---
module_id: "debug"
title: "Debug Utilities — debug.c & debug.h"
difficulty: "Moderate"
prerequisites:
  - "utilities-overview"
key_concepts:
  - "ASSERT macro"
  - "PANIC"
  - "Backtraces"
estimated_reading_time: "5 min"
source_files:
  - "pintos/src/lib/debug.c"
  - "pintos/src/lib/debug.h"
---

# Debug Utilities — debug.c & debug.h

## 1. Executive Summary

`debug.h` defines the single most important macro in Pintos: `ASSERT`. It also provides `PANIC`, `NOT_REACHED`, and `NO_RETURN`.

## 2. Key Macros

- **`ASSERT(EXPR)`**: If `EXPR` is false, prints the file, line, and expression, then calls `PANIC`. Compiled out in release builds (`NDEBUG`).
- **`PANIC(FORMAT, ...)`**: Prints the message, dumps the call stack (backtrace), and halts the kernel. There is no recovery.
- **`NOT_REACHED()`**: Placed after code that should never execute (e.g., after an infinite loop or a `switch` that handles all cases). Equivalent to `PANIC("NOT_REACHED")`.
- **`NO_RETURN`**: Attribute marking a function that never returns (like `thread_exit`).

### Backtraces

When `PANIC` fires, it prints a list of return addresses from the call stack. You can decode these using the `backtrace` utility:
```bash
backtrace build/kernel.o 0xc0106b38 0xc010dc22 ...
```
This gives you the actual function names and line numbers.

## 3. Lab Modifications Guide

This file is **not modified in any lab**. Use `ASSERT` liberally in your own code to catch bugs early.
