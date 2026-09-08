---
module_id: "arithmetic"
title: "Arithmetic Helpers — arithmetic.h"
difficulty: "Light"
prerequisites: []
key_concepts:
  - "DIV_ROUND_UP"
  - "ROUND_UP, ROUND_DOWN"
estimated_reading_time: "2 min"
source_files:
  - "pintos/src/lib/arithmetic.c"
  - "pintos/src/lib/round.h"
---

# Arithmetic Helpers — arithmetic.h & round.h

## 1. Executive Summary

Provides integer rounding macros used throughout the kernel:

- **`DIV_ROUND_UP(X, STEP)`**: Divides X by STEP, rounding up. Used to calculate how many pages a given byte size needs.
- **`ROUND_UP(X, STEP)`**: Rounds X up to the nearest multiple of STEP.
- **`ROUND_DOWN(X, STEP)`**: Rounds X down to the nearest multiple of STEP.

Also provides `lib/arithmetic.c` which currently contains no functions (placeholder for potential future fixed-point math).

## 2. Lab Modifications Guide

These files are **not modified in any lab**. You may use `DIV_ROUND_UP` when calculating page counts.
