---
module_id: "test-infrastructure"
title: "Test Infrastructure — tests/lib/"
difficulty: "Light"
prerequisites:
  - "testing-overview"
key_concepts:
  - "Test helper functions"
  - "msg(), fail()"
estimated_reading_time: "3 min"
source_files:
  - "pintos/tests/lib.c"
  - "pintos/tests/lib.h"
  - "pintos/tests/main.c"
  - "pintos/tests/main.h"
---

# Test Infrastructure — tests/lib/

## 1. Executive Summary

The test infrastructure provides helper functions that all test programs use to produce consistent, machine-parseable output.

## 2. Key Functions

- **`msg(format, ...)`**: Like `printf`, but prefixes output with the test name in parentheses. The `.ck` grading scripts match against this format.
- **`fail(format, ...)`**: Prints a FAIL message and calls `exit(1)`.
- **`pass()`**: Prints "PASS" and exits successfully.
- **`test_main()`**: Each test defines this function. The `main()` in `tests/main.c` calls it after printing a "begin" header.

### `arc4.c` and `cksum.c`

Utility functions for computing checksums and generating pseudo-random data used by some file system tests to verify data integrity.

## 3. Lab Modifications Guide

These files are **not modified in any lab**. Do not modify test files.
