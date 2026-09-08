---
module_id: "helper-tools"
title: "Helper Tools — backtrace, pintos-gdb, pintos-mkdisk"
difficulty: "Light"
prerequisites:
  - "tools-overview"
key_concepts:
  - "Debugging with GDB"
  - "Creating disk images"
  - "Decoding backtraces"
estimated_reading_time: "5 min"
source_files:
  - "pintos/utils/backtrace"
  - "pintos/utils/pintos-gdb"
  - "pintos/utils/pintos-mkdisk"
---

# Helper Tools — backtrace, pintos-gdb, pintos-mkdisk

## 1. Executive Summary

### `backtrace`
When your kernel PANICs, it dumps a series of hex addresses. The `backtrace` utility translates these into human-readable function names and line numbers:
```bash
backtrace build/kernel.o 0xc0106b38 0xc010dc22 0xc010dc36
```
Output: `0xc0106b38: debug_panic (lib/debug.c:86)` etc.

### `pintos-gdb`
A wrapper around GDB pre-configured for Pintos. To use it:
1. Start Pintos with `pintos --gdb -- run test-name`.
2. In another terminal: `pintos-gdb build/kernel.o`.
3. In GDB: `debugpintos` to connect to the emulator.

Useful GDB commands for Pintos:
- `dumplist &all_list thread allelem`: Print all threads.
- `btpagefault`: Print the backtrace of the last page fault.
- `loadusersymbols build/tests/userprog/args-single`: Load user program debug symbols.

### `pintos-mkdisk`
Creates a blank virtual disk image of a specified size:
```bash
pintos-mkdisk filesys.dsk --filesys-size=2
```

## 2. Lab Modifications Guide

These tools are **not modified in any lab**. They are essential debugging aids you should learn to use.
