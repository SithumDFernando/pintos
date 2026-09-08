---
module_id: "stdlib"
title: "Standard Library — stdlib.h"
difficulty: "Light"
prerequisites:
  - "utilities-overview"
key_concepts:
  - "atoi"
  - "strtol"
  - "qsort, bsearch"
estimated_reading_time: "3 min"
source_files:
  - "pintos/src/lib/stdlib.h"
  - "pintos/src/lib/stdlib.c"
---

# Standard Library — stdlib.h

## 1. Executive Summary

Provides a subset of standard C library functions: `atoi()` for string-to-integer conversion, `strtol()` for more robust parsing, and `qsort()`/`bsearch()` for array operations.

## 2. Lab Modifications Guide

This file is **not modified in any lab**. You may use `atoi` when parsing command-line arguments in P0.
