---
module_id: "utilities-overview"
title: "Utilities & Data Structures — Overview"
difficulty: "Beginner"
prerequisites:
  - "threading-overview"
key_concepts:
  - "Kernel Library"
  - "Standard C functions"
estimated_reading_time: "5 min"
source_files:
  - "pintos/src/lib/"
---

# Utilities & Data Structures — Overview

## 1. Executive Summary & Mental Model

Pintos does not run on top of Linux; it *is* an operating system. This means you do not have access to the standard GNU C Library (`glibc`). You cannot simply `#include <stdlib.h>` and expect it to link against your host OS's libraries.

The `lib/` directory provides a minimalistic, custom-built standard library specifically for Pintos. 
It contains implementations of `printf`, `memcpy`, `strlen`, and custom data structures like Linked Lists and Bitmaps.

## 2. Directory Structure

- `lib/`: Standard C header files (`stdbool.h`, `stdint.h`, `string.h`) and their implementations (`string.c`).
- `lib/kernel/`: Code that can *only* be used inside the kernel (e.g., `list.c`, `bitmap.c`, `hash.c`, `console.c`).
- `lib/user/`: Code that is compiled into user programs (e.g., the syscall stubs in `syscall.c`).

## 3. The `ASSERT` Macro

You will see `ASSERT (condition);` everywhere in the Pintos codebase. 
If the condition evaluates to `false`, the kernel immediately PANICs, printing the file name and line number. 

**Use assertions liberally in your own code** to catch bugs early. If you write a function that expects a non-null pointer, start it with `ASSERT (ptr != NULL);`.

## 4. Lab Modifications Guide

You will **not modify** the files in this directory. However, you will use them extensively. You must understand how `list.c` works to pass Project 1 and Project 2.
