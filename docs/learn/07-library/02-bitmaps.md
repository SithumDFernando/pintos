---
module_id: "bitmaps"
title: "Bitmaps — bitmap.c"
difficulty: "Light"
prerequisites:
  - "utilities-overview"
key_concepts:
  - "Bit manipulation"
  - "Resource tracking"
estimated_reading_time: "3 min"
source_files:
  - "pintos/src/lib/kernel/bitmap.c"
---

# Bitmaps — bitmap.c

## 1. Executive Summary & Mental Model

A bitmap is a highly memory-efficient array of booleans. Each boolean value takes up exactly one bit (0 or 1). It is used primarily to track the availability of resources (like memory pages or disk sectors).

## 2. Source Code Anatomy & Deep Dive

The `struct bitmap` contains a pointer to an array of `unsigned long` integers. 
If an `unsigned long` is 32 bits, then a single integer can track 32 resources.

- `bitmap_create(bit_cnt)`: Allocates a new bitmap.
- `bitmap_mark(b, bit_idx)`: Sets a bit to 1 (true).
- `bitmap_reset(b, bit_idx)`: Sets a bit to 0 (false).
- `bitmap_test(b, bit_idx)`: Returns the value of a bit.
- `bitmap_scan_and_flip(b, start, cnt, value)`: Finds `cnt` contiguous bits that have the given `value`, flips them to the opposite value, and returns the starting index. This is used by `palloc.c` to find free memory pages.

## 3. Lab Modifications Guide

This file is **not modified in any lab**. You may find it useful if you need to track a large number of true/false values, but for Labs 1 and 2, you generally won't need to interact with it directly.
