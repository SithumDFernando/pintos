---
module_id: "string"
title: "String Functions — string.h & string.c"
difficulty: "Moderate"
prerequisites:
  - "utilities-overview"
key_concepts:
  - "memcpy, memset, memmove"
  - "strlcpy, strlcat"
  - "strtok_r"
estimated_reading_time: "5 min"
source_files:
  - "pintos/src/lib/string.h"
  - "pintos/src/lib/string.c"
---

# String Functions — string.h & string.c

## 1. Executive Summary

Provides standard memory and string manipulation functions. The most important for your labs:

- **`strlcpy(dst, src, size)`**: Safe string copy. Always null-terminates. Prevents buffer overflows (unlike `strcpy`).
- **`strtok_r(str, delimiters, &save_ptr)`**: Tokenizes a string by splitting on delimiter characters. The `_r` suffix means it is **reentrant** (thread-safe). You will use this in P2 to parse command-line arguments like `"echo hello world"` into `["echo", "hello", "world"]`.
- **`memcpy(dst, src, size)`**: Copies `size` bytes. Used extensively when pushing arguments onto the user stack.
- **`memset(dst, value, size)`**: Fills memory with a byte value. Used for zeroing pages.

## 2. Lab Modifications Guide

This file is **not modified in any lab**. You will use `strtok_r` heavily in P2 for argument parsing.
