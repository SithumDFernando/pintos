---
module_id: "shutdown"
title: "System Shutdown — shutdown.c"
difficulty: "Light"
prerequisites:
  - "io-ports"
key_concepts:
  - "ACPI power off"
  - "System reboot"
estimated_reading_time: "3 min"
source_files:
  - "pintos/src/devices/shutdown.c"
---

# System Shutdown — shutdown.c

## 1. Executive Summary & Mental Model

This tiny module handles turning off the emulated computer or rebooting it. It provides the standard `shutdown()` function called at the very end of `pintos_init()`.

## 2. Source Code Anatomy & Deep Dive

- **`shutdown_power_off()`**: Turns off the computer. In QEMU and Bochs, this is achieved by writing a magic string (e.g., "Shutdown") or specific bytes to a specific APM/ACPI hardware port. This tells the emulator to kill its own process.
- **`shutdown_reboot()`**: Reboots the machine. It does this by pulsing the reset line on the keyboard controller (a classic x86 hack) using Port I/O (`outb(0x64, 0xfe)`).

## 3. Lab Modifications Guide

This file is **not modified in any lab**.
