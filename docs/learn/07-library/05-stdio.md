---
module_id: "stdio"
title: "Standard I/O — stdio.h & stdio.c"
difficulty: "Moderate"
prerequisites:
  - "utilities-overview"
key_concepts:
  - "printf implementation"
  - "vprintf, snprintf"
  - "hex_dump"
estimated_reading_time: "5 min"
source_files:
  - "pintos/src/lib/stdio.h"
  - "pintos/src/lib/stdio.c"
  - "pintos/src/lib/kernel/console.c"
---

# Standard I/O — stdio.h & stdio.c

## 1. Executive Summary

Pintos implements its own `printf` family of functions since there is no standard C library. The implementation supports `%d`, `%u`, `%x`, `%s`, `%c`, `%p`, and width/precision modifiers.

## 2. Key Functions

- **`printf(format, ...)`**: Writes formatted output to the console (VGA + serial port). Thread-safe via a console lock.
- **`snprintf(buf, size, format, ...)`**: Writes formatted output into a buffer (useful for building strings).
- **`putchar(c)`**: Writes a single character to the console.
- **`puts(s)`**: Writes a string followed by a newline.
- **`hex_dump(ofs, buf, size, ascii)`**: Prints a hex dump of memory. Invaluable for debugging stack contents and memory layouts.
- **`putbuf(buf, size)`**: Writes `size` bytes from `buf` to the console. Used by `SYS_WRITE` on fd 1.

### Console Lock (`console.c`)

`lib/kernel/console.c` wraps `printf` with a lock to prevent garbled output when multiple threads print simultaneously. It also routes output to both VGA and the serial port.

## 3. Lab Modifications Guide

These files are **not modified in any lab**. You will call `printf` for debugging and `putbuf` in your `SYS_WRITE` handler for fd 1 (stdout).
