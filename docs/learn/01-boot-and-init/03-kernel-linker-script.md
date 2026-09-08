---
module_id: "boot-linker-script"
title: "Kernel Linker Script — kernel.lds.S"
difficulty: "Intermediate"
prerequisites:
  - "boot-start"
  - "overview-build-system"
key_concepts:
  - "Linker scripts"
  - "ELF sections (.text, .rodata, .data, .bss)"
  - "Kernel memory layout"
  - "Load address vs virtual address"
estimated_reading_time: "8 min"
source_files:
  - "pintos/src/threads/kernel.lds.S"
---

# Kernel Linker Script — kernel.lds.S

## 1. Executive Summary & Mental Model

The linker script tells `ld` (the linker) **where to place each section** of the kernel in memory. Without it, the linker would use default addresses that don't match Pintos's boot expectations.

**Analogy**: The linker script is a **floor plan** for the kernel's memory. It says "code goes at address X, data at Y, leave room for ELF headers at the front."

## 2. Theoretical Foundations

### ELF Sections

| Section | Contains | Properties |
|---|---|---|
| `.text` | Machine code (compiled functions) | Executable, read-only |
| `.rodata` | Read-only data (string literals, constants) | Not executable, read-only |
| `.data` | Initialized globals/statics | Readable, writable |
| `.bss` | Uninitialized globals/statics | Readable, writable, not stored on disk |

### Virtual vs Physical Address

The kernel is **loaded** at physical address `0x20000` (128 KB) but **linked** to run at virtual address `0xC0020000` (3 GB + 128 KB). The page tables set up by `start.S` and `paging_init()` create the mapping between these.

## 3. Architecture & Visual Topography

### Kernel Memory Layout

```
Virtual Address Space:
0xC0020000  +------------------+  ← _start (LOADER_PHYS_BASE + LOADER_KERN_BASE)
            | ELF Headers      |
            +------------------+
            | .start section   |  ← start.S code (first thing to run)
            | .text section    |  ← All compiled kernel functions
            +------------------+
            | .rodata section  |  ← String literals, constants
            +------ 4KB aligned +
            | _end_kernel_text |  ← Boundary: above is read-only, below is writable
            +------------------+
            | .data section    |  ← Initialized global variables
            | 0xAA55AA55       |  ← _signature (helps detect kernel in memory)
            +------------------+
            | .bss section     |  ← _start_bss to _end_bss (zeroed by bss_init)
            +------------------+  ← _end
            | (free memory)    |
            +------------------+

ASSERT: _end - _start ≤ 512K
```

## 4. Source Code Anatomy & Deep Dive

```c
#include "threads/loader.h"       // Gets LOADER_PHYS_BASE and LOADER_KERN_BASE

OUTPUT_FORMAT("elf32-i386")       // Output 32-bit x86 ELF
OUTPUT_ARCH("i386")               // Target architecture
ENTRY(start)                      // Kernel entry point (start label in start.S)

SECTIONS
{
  /* Kernel virtual base address */
  _start = LOADER_PHYS_BASE + LOADER_KERN_BASE;    // 0xC0000000 + 0x20000 = 0xC0020000

  /* Leave room for ELF headers */
  . = _start + SIZEOF_HEADERS;

  /* Code section: .start first (start.S), then all other .text */
  .text : { *(.start) *(.text) } = 0x90            // 0x90 = NOP for padding

  /* Read-only data, aligned to 4KB boundary */
  .rodata : { *(.rodata) *(.rodata.*)
              . = ALIGN(0x1000);
              _end_kernel_text = .; }               // Mark end of read-only region

  /* Initialized data + kernel signature */
  .data : { *(.data)
            _signature = .; LONG(0xaa55aa55) }      // Embedded signature

  /* BSS (uninitialized data) — boundaries exported as symbols */
  _start_bss = .;
  .bss : { *(.bss) }
  _end_bss = .;

  _end = .;                                         // End of kernel image

  ASSERT (_end - _start <= 512K, "Kernel image is too big.")
}
```

### Key Symbols Exported

| Symbol | Used By | Purpose |
|---|---|---|
| `_start` | Linker | Kernel base virtual address |
| `_end_kernel_text` | `paging_init()` | Separates read-only code from writable data for page permissions |
| `_start_bss`, `_end_bss` | `bss_init()` | Boundaries of BSS segment to be zeroed |
| `_end` | Various | End of kernel image |
| `_signature` | Debugging | `0xAA55AA55` helps identify kernel in memory dumps |

## 5. Runtime Execution & Sequence

The linker script runs at **build time**, not runtime. It produces the `kernel.o` and `kernel.bin` files with correct address assignments. At runtime, the kernel executes at these addresses because `start.S` creates matching page table entries.

## 6. Edge Cases, Failure Modes & Resilience

| Issue | What Happens |
|---|---|
| Kernel code exceeds 512 KB | `ASSERT` fails during linking with "Kernel image is too big" |
| `.start` section missing | Entry point `start` not found → linker error |
| Alignment not done for `.rodata` | `_end_kernel_text` misaligned → `paging_init()` may set wrong page permissions |

## 7. Hands-On Verification & Challenges

<details>
<summary><strong>Q1: Why is .start placed before .text?</strong></summary>

The `start` function in `start.S` must be the first code in the kernel image. The bootloader jumps to the ELF entry point, which is `start`. By placing `.start` section first with `*(.start) *(.text)`, the linker ensures `start` is at the beginning of the code section, right after the ELF headers.
</details>

## 8. Lab Modifications Guide

This file is **not modified in any lab**. If your kernel grows too large (unlikely in P1/P2), you'll hit the 512 KB assertion — reduce code or use dynamic allocation.
