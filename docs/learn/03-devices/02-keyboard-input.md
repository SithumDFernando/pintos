---
module_id: "keyboard-input"
title: "Keyboard Input — kbd.c & input.c"
difficulty: "Light"
prerequisites:
  - "device-drivers-overview"
key_concepts:
  - "PS/2 Keyboard Controller"
  - "Hardware interrupts"
  - "Input buffering"
estimated_reading_time: "5 min"
source_files:
  - "pintos/src/devices/kbd.c"
  - "pintos/src/devices/input.c"
---

# Keyboard Input — kbd.c & input.c

## 1. Executive Summary & Mental Model

These files handle reading physical keystrokes from the keyboard and buffering them until a thread asks for them (e.g., when a user program calls the `read` system call on `STDIN`).

**Real-world analogy**:
The keyboard is a conveyor belt of packages (keys). 
- `kbd.c` is the worker at the end of the belt picking up the packages as they arrive.
- `input.c` is the warehouse shelf where the packages are stored until a delivery truck (user program) comes to pick them up.

## 2. Source Code Anatomy & Deep Dive

### `kbd.c` (Hardware Layer)

The PS/2 keyboard triggers an interrupt (IRQ 1, vector `0x21`) every time a key is pressed *or released*. 
- `kbd_init()`: Registers the interrupt handler `keyboard_interrupt`.
- `keyboard_interrupt()`: Reads the raw "scancode" from I/O port `0x60` (the keyboard data port).
- The handler translates the raw scancodes (make/break codes) into standard ASCII characters, handling Shift, Caps Lock, and Alt modifiers.
- Once a valid ASCII character is decoded, it passes it to `input.c` by calling `input_putc(key)`.

### `input.c` (Buffer Layer)

This file maintains a circular buffer (a queue) of size `intq_buf` (usually 64 characters) to hold typed characters.

- `input_putc(key)`: Pushes the character into the buffer. If the buffer is full, the character is dropped.
- `input_getc()`: Pops a character from the buffer. If the buffer is empty, **it blocks the calling thread** until a character arrives. It uses `intr_disable()` and `thread_block()` to wait, or uses specialized synchronization in later projects.

## 3. Runtime Sequence

1. User presses 'A'.
2. Keyboard hardware sends IRQ 1.
3. CPU jumps to `keyboard_interrupt()`.
4. Handler reads port `0x60`, gets scancode for 'A'.
5. Handler calls `input_putc('A')`.
6. 'A' is placed in the buffer. If a thread was blocked in `input_getc()`, it is unblocked.

## 4. Lab Modifications Guide

These files are **not modified in any lab**. 
However, in **Project 2 (User Programs)**, you must route the `read` system call for file descriptor 0 (`STDIN_FILENO`) to `input_getc()` so user programs can read keyboard input!
