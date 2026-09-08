---
module_id: "random"
title: "Random Number Generator — random.c"
difficulty: "Light"
prerequisites: []
key_concepts:
  - "Pseudo-random number generation"
estimated_reading_time: "2 min"
source_files:
  - "pintos/src/lib/random.c"
  - "pintos/src/lib/random.h"
---

# Random Number Generator — random.c

## 1. Executive Summary

Provides a deterministic pseudo-random number generator seeded during boot from the RTC. Uses a simple linear congruential generator.

- `random_init(seed)`: Seeds the generator.
- `random_ulong()`: Returns a random `unsigned long`.
- `random_bytes(buf, size)`: Fills a buffer with random bytes.
- `shuffle(buf, cnt, size)`: Fisher-Yates shuffle of an array.

Used by some test cases and by the scheduler for tie-breaking.

## 2. Lab Modifications Guide

This file is **not modified in any lab**.
