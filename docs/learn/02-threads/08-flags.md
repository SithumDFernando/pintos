---
module_id: "flags"
title: "EFLAGS Register — flags.h"
difficulty: "Light"
prerequisites:
  - "interrupts"
key_concepts:
  - "EFLAGS register"
  - "Interrupt Enable Flag (IF)"
estimated_reading_time: "3 min"
source_files:
  - "pintos/src/threads/flags.h"
---

# EFLAGS Register — flags.h

## 1. Executive Summary & Mental Model

`flags.h` is a tiny header file containing bitmask definitions for the x86 `EFLAGS` register. 

The `EFLAGS` register is a special 32-bit register inside the CPU that stores the current state of the processor, including the results of math operations (zero, carry, overflow) and system control flags (like whether interrupts are enabled).

## 2. Source Code Anatomy & Deep Dive

```c
#ifndef THREADS_FLAGS_H
#define THREADS_FLAGS_H

/* EFLAGS Register. */
#define FLAG_MBS  0x00000002    /* Must be set. */
#define FLAG_IF   0x00000200    /* Interrupt Flag. */

#endif /* threads/flags.h */
```

### The Interrupt Flag (`FLAG_IF`)

This is the only flag Pintos actively checks via software. Bit 9 of the `EFLAGS` register is the **Interrupt Flag (IF)**.

- **IF = 1**: The CPU responds to external hardware interrupts (like the timer or keyboard).
- **IF = 0**: The CPU ignores external hardware interrupts (they remain pending).

When you call `intr_disable()`, the CPU executes the `cli` instruction, which clears this bit to 0. 
When you call `intr_enable()`, the CPU executes the `sti` instruction, which sets this bit to 1.

Pintos uses `FLAG_IF` in `intr_get_level()`:
```c
enum intr_level intr_get_level (void) {
  uint32_t flags;
  // Pushes EFLAGS to stack, pops into 'flags' variable
  asm volatile ("pushfl; popl %0" : "=g" (flags));
  
  // Mask against FLAG_IF to check if bit 9 is 1
  return flags & FLAG_IF ? INTR_ON : INTR_OFF;
}
```
This allows the kernel to query whether interrupts are currently on or off without changing them.

### `FLAG_MBS`

Bit 1 of `EFLAGS` is hardwired to 1 by Intel. It is labeled "Must Be Set". Pintos defines it for completeness but doesn't actively use it.

## 3. Lab Modifications Guide

This file is **not modified in any lab**. It exists solely to provide the `FLAG_IF` constant to `interrupt.c`.
