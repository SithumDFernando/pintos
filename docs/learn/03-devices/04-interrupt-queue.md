---
module_id: "interrupt-queue"
title: "Interrupt Queue — intq.c"
difficulty: "Light"
prerequisites:
  - "keyboard-input"
  - "serial-port"
key_concepts:
  - "Circular buffer"
  - "Producer-Consumer problem"
  - "Interrupt-safe synchronization"
estimated_reading_time: "5 min"
source_files:
  - "pintos/src/devices/intq.c"
---

# Interrupt Queue — intq.c

## 1. Executive Summary & Mental Model

The `intq` is a specialized circular buffer (queue) designed specifically for transferring data between **interrupt handlers** (which cannot sleep or wait on semaphores) and **kernel threads** (which can). 

It is the core data structure backing both keyboard input (`input.c`) and serial port output (`serial.c`).

## 2. Source Code Anatomy & Deep Dive

An `intq` holds up to `INTQ_BUFSIZE` (64) bytes. 
It uses the classic Producer-Consumer pattern with two pointers: `head` (where data is added) and `tail` (where data is removed).

### Key Functions

- `intq_putc()`: Adds a byte to the queue. If the queue is full, it drops the byte. If a thread is waiting in `intq_getc()`, it unblocks it using a lock.
- `intq_getc()`: Removes a byte. If the queue is empty, it uses `lock_acquire()` and `cond_wait()` to sleep until a byte arrives.

### Synchronization Trick

You cannot use standard locks inside an interrupt handler. So how does `intq_putc()` (called by the keyboard interrupt) wake up `intq_getc()` safely?
It disables interrupts and checks if there's a waiting thread. It doesn't actually use a `struct lock` or `cond_signal` inside the interrupt context, because those require holding a lock. Instead, the `intq` uses a custom synchronization mechanism (often just disabling interrupts) to ensure the `head` and `tail` pointers aren't corrupted by concurrent access.

## 3. Lab Modifications Guide

This file is **not modified in any lab**.
