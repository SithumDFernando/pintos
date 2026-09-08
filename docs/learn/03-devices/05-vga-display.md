---
module_id: "vga-display"
title: "VGA Display — vga.c"
difficulty: "Light"
prerequisites:
  - "device-drivers-overview"
key_concepts:
  - "Memory-mapped I/O"
  - "VGA text mode"
estimated_reading_time: "3 min"
source_files:
  - "pintos/src/devices/vga.c"
---

# VGA Display — vga.c

## 1. Executive Summary & Mental Model

The VGA driver handles printing text to the screen (the QEMU window). Unlike the serial port or keyboard which use Port-Mapped I/O, the VGA display uses **Memory-Mapped I/O**.

## 2. Source Code Anatomy & Deep Dive

In standard PC architecture, physical memory starting at `0xB8000` is mapped directly to the video card's text buffer. 

The buffer is an array of 16-bit values, representing an 80x25 grid of characters.
- **Lower 8 bits**: The ASCII character (e.g., 'A').
- **Upper 8 bits**: The color attribute (e.g., light gray text on black background).

### Writing to the Screen
When you call `printf()`, it eventually calls `vga_putc()`. The driver simply writes the character to the correct memory address at `ptov(0xB8000) + offset`.

### Hardware Cursor
The blinking cursor is controlled by the VGA controller chip, not by memory. The driver must send specific commands via Port I/O (ports `0x3D4` and `0x3D5`) to tell the hardware which (x, y) coordinate should blink.

## 3. Lab Modifications Guide

This file is **not modified in any lab**.
