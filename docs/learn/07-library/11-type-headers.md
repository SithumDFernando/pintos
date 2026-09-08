---
module_id: "type-headers"
title: "Type Headers — stdint.h, stddef.h, stdbool.h, stdarg.h, etc."
difficulty: "Light"
prerequisites: []
key_concepts:
  - "Fixed-width integer types"
  - "Freestanding C headers"
estimated_reading_time: "3 min"
source_files:
  - "pintos/src/lib/stdint.h"
  - "pintos/src/lib/stddef.h"
  - "pintos/src/lib/stdbool.h"
  - "pintos/src/lib/stdarg.h"
  - "pintos/src/lib/inttypes.h"
  - "pintos/src/lib/packed.h"
---

# Type Headers — stdint.h, stddef.h, stdbool.h, etc.

## 1. Executive Summary

These headers define fundamental C types that the kernel relies on. Since Pintos cannot use the host system's headers, it provides its own freestanding implementations.

| Header | Defines |
|---|---|
| `stdint.h` | `int8_t`, `int16_t`, `int32_t`, `int64_t`, `uint8_t`, etc. Fixed-width integers. |
| `stddef.h` | `size_t`, `NULL`, `offsetof()` macro (critical for `list_entry`). |
| `stdbool.h` | `bool`, `true`, `false`. |
| `stdarg.h` | `va_list`, `va_start`, `va_arg`, `va_end`. For variadic functions like `printf`. Uses GCC builtins. |
| `inttypes.h` | `PRId32`, `PRIu64`, etc. Format specifiers for `printf` with fixed-width types. |
| `packed.h` | `PACKED` attribute macro. Prevents struct padding for hardware-mapped structures. |

## 2. Lab Modifications Guide

These files are **not modified in any lab**.
