---
module_id: "ide-disk"
title: "IDE Disk Controller — ide.c"
difficulty: "Light"
prerequisites:
  - "block-devices"
  - "io-ports"
key_concepts:
  - "ATA / IDE interface"
  - "Interrupt-driven disk I/O"
estimated_reading_time: "5 min"
source_files:
  - "pintos/src/devices/ide.c"
---

# IDE Disk Controller — ide.c

## 1. Executive Summary & Mental Model

The IDE driver is responsible for actually reading and writing to the emulated hard drives in Pintos. It translates the abstract `block_read()` requests into low-level port I/O commands sent to the IDE controller chip.

## 2. Source Code Anatomy & Deep Dive

The IDE driver communicates with disks using the ATA standard.
- It writes the target sector number (LBA - Logical Block Address) to a series of specific I/O ports.
- It sends a "Read Sector" or "Write Sector" command.

### Waiting for Data
Disks are extremely slow compared to the CPU. 
1. The driver sends the read command.
2. It calls `sema_down()` to block the current thread.
3. The disk seeks to the sector, reads the data, and places it in an internal buffer.
4. The disk fires a hardware interrupt (IRQ 14 or 15).
5. The `ide_interrupt()` handler reads the data from the disk's buffer using `insw()` (Port I/O) and calls `sema_up()` to wake the blocked thread.

This interrupt-driven approach ensures the CPU can run other threads while waiting for the disk to spin!

## 3. Lab Modifications Guide

This file is **not modified in any lab**.
