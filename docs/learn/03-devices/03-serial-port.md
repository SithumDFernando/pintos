---
module_id: "serial-port"
title: "Serial Port — serial.c"
difficulty: "Light"
prerequisites:
  - "device-drivers-overview"
key_concepts:
  - "UART 8250"
  - "Polling vs Interrupt-driven output"
estimated_reading_time: "5 min"
source_files:
  - "pintos/src/devices/serial.c"
---

# Serial Port — serial.c

## 1. Executive Summary & Mental Model

The serial port (COM1) is the primary way Pintos communicates with the outside world during development. In the QEMU emulator, everything sent to the serial port is printed to your host terminal. It is used extensively by `printf()`.

## 2. Source Code Anatomy & Deep Dive

The serial port uses a UART chip, controlled via I/O ports starting at `0x3f8`.

### Two Modes of Operation

1. **Polling Mode (Early Boot)**:
   When Pintos first boots, interrupts are disabled. `serial_putc()` uses a busy-wait loop, constantly reading the UART status register to see if it's ready to transmit the next character. This is slow but necessary before the interrupt system is initialized.

2. **Interrupt-Driven Mode (Runtime)**:
   Later in `pintos_init()`, `serial_init_queue()` is called. This switches the driver to use an interrupt-driven queue.
   - When you call `serial_putc()`, the character is placed in a transmission buffer (an `intq`).
   - If the UART is idle, it kicks off transmission.
   - When the UART finishes sending a character, it fires IRQ 4 (vector `0x24`).
   - The interrupt handler `serial_interrupt()` grabs the next character from the buffer and sends it.

## 3. Lab Modifications Guide

This file is **not modified in any lab**. You interact with it implicitly every time you use `printf()`, `msg()`, or `debug()`.
