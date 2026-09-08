---
module_id: "io-ports"
title: "I/O Ports & Hardware Communication — io.h"
difficulty: "Light"
prerequisites:
  - "boot-sequence-overview"
key_concepts:
  - "Memory-mapped I/O vs Port-mapped I/O"
  - "x86 IN and OUT instructions"
  - "Inline assembly"
estimated_reading_time: "5 min"
source_files:
  - "pintos/src/threads/io.h"
---

# I/O Ports & Hardware Communication — io.h

## 1. Executive Summary & Mental Model

How does the CPU actually talk to hardware like the hard drive, the keyboard, or the serial port? In the x86 architecture, it uses **Port-Mapped I/O**. `io.h` provides the C wrappers for the special assembly instructions needed to read and write to these hardware ports.

**Real-world analogy**: 
Think of the CPU as an office worker at a desk. 
- Writing to RAM is like putting papers in filing cabinets (Memory). 
- Writing to an I/O port is like dropping a paper into a pneumatic tube (Port) that zips directly to another department (Hardware Device).

## 2. Theoretical Foundations

### Port-Mapped I/O

The x86 architecture has a completely separate address space for hardware devices, spanning from port `0x0000` to `0xFFFF` (65,536 ports). 
Standard memory instructions (like `MOV`) cannot access these ports. You must use the `IN` instruction to read from a port and the `OUT` instruction to write to it.

For example, the serial port (COM1) is traditionally mapped to ports starting at `0x3F8`. The keyboard controller uses ports `0x60` and `0x64`.

## 3. Source Code Anatomy & Deep Dive

`io.h` uses GCC inline assembly to wrap the `IN` and `OUT` instructions so they can be called like normal C functions.

### Reading from Ports
- `inb(port)`: Read 1 byte (8 bits) from `port`.
- `inw(port)`: Read 1 word (16 bits) from `port`.
- `inl(port)`: Read 1 double-word (32 bits) from `port`.

```c
static inline uint8_t inb (uint16_t port) {
  uint8_t data;
  /* Execute "inb %dx, %al" where %dx is 'port' and %al is 'data' */
  asm volatile ("inb %w1, %b0" : "=a" (data) : "Nd" (port));
  return data;
}
```

### Writing to Ports
- `outb(port, data)`: Write 1 byte to `port`.
- `outw(port, data)`: Write 1 word to `port`.
- `outl(port, data)`: Write 1 double-word to `port`.

### Bulk Transfers
Sometimes you need to read an entire sector from a disk (512 bytes). Doing a loop of `inb` would be slow. The x86 has repeating string instructions (`rep insb`) that transfer bulk data directly to memory.
- `insb(port, addr, cnt)`: Reads `cnt` bytes from `port` directly into memory at `addr`.

## 4. Edge Cases, Failure Modes & Resilience

| Scenario | Consequence |
|---|---|
| User process tries to call `inb()` | General Protection Fault. By default, user programs do not have the privilege to execute I/O instructions (I/O Privilege Level is 0). |
| Writing to a random, unused port | Usually ignored by the hardware, but occasionally can cause system freezes if it hits an undocumented legacy device. |

## 5. Lab Modifications Guide

This file is **not modified in any lab**. You will likely not need to call these functions directly unless you are writing a custom device driver. The existing drivers in the `devices/` directory (like `serial.c` and `ide.c`) use these functions heavily.
