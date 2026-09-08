---
module_id: "block-devices"
title: "Block Devices — block.c"
difficulty: "Light"
prerequisites:
  - "device-drivers-overview"
key_concepts:
  - "Block device abstraction"
  - "Sectors"
estimated_reading_time: "3 min"
source_files:
  - "pintos/src/devices/block.c"
---

# Block Devices — block.c

## 1. Executive Summary & Mental Model

A block device is any storage medium that reads and writes data in fixed-size chunks called **sectors** (always 512 bytes in Pintos), rather than one character at a time. Hard drives, SSDs, and USB drives are block devices.

`block.c` provides an abstract interface for block devices. It allows the kernel's File System to say "Read sector 5" without caring whether sector 5 is on an IDE hard drive, a partition of a drive, or a RAM disk.

## 2. Source Code Anatomy & Deep Dive

The core abstraction is `struct block`, which contains function pointers:
```c
struct block_operations {
    void (*read) (void *aux, block_sector_t, void *buffer);
    void (*write) (void *aux, block_sector_t, const void *buffer);
};
```
This is a basic implementation of Object-Oriented polymorphism in C. The IDE driver provides its own `read` and `write` functions, which `block.c` calls when requested.

The Pintos file system uses this layer exclusively. It never interacts with the IDE driver directly.

## 3. Lab Modifications Guide

This file is **not modified in any lab**. You will interact with it indirectly in **Project 4 (File Systems)** when you implement a buffer cache, but you won't change this file itself.
