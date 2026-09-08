---
module_id: "internal-tests"
title: "Internal Tests — tests/internal/"
difficulty: "Light"
prerequisites:
  - "testing-overview"
key_concepts:
  - "Kernel-level tests"
  - "List and stdio tests"
estimated_reading_time: "2 min"
source_files:
  - "pintos/tests/internal/"
---

# Internal Tests — tests/internal/

## 1. Executive Summary

The `tests/internal/` directory contains tests that run inside the kernel (not as user programs). They verify that kernel-level data structures and functions work correctly.

| Test | What It Checks |
|---|---|
| `list` | Tests the intrusive linked list implementation (`list.c`). |
| `stdio` | Tests `printf` formatting. |
| `stdlib` | Tests `strtol()` and other stdlib functions. |

These tests run by specifying `-T` (internal test) when invoking `pintos`.

## 2. Lab Modifications Guide

These files are **not modified in any lab**.
