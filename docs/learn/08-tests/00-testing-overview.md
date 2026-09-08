---
module_id: "testing-overview"
title: "Testing Infrastructure — tests/"
difficulty: "Beginner"
prerequisites:
  - "build-system"
key_concepts:
  - "Automated testing"
  - "Perl scripts"
estimated_reading_time: "5 min"
source_files:
  - "pintos/tests/"
---

# Testing Infrastructure — tests/

## 1. Executive Summary & Mental Model

Pintos comes with a massive, robust automated test suite. It is how your code is graded. 

When you run `make check` inside `build/`, Pintos compiles the kernel, compiles the user test programs, formats a virtual disk, copies the test program into the virtual disk, boots the emulator (QEMU), runs the test program, captures the serial output, and uses a Perl script to compare the output against an expected result.

## 2. Deep Dive into a Test

Every test has three components (e.g., for the `alarm-single` test in Project 1):
1. **The C Code**: `tests/threads/alarm-single.c`. This is the code that is actually executed.
2. **The Expected Output**: `tests/threads/alarm-single.ck`. A `.ck` file is a perl script that defines regex patterns to match against your kernel's output.
3. **The Result**: When you run the test, the build system creates `tests/threads/alarm-single.result` (PASS or FAIL), `alarm-single.output` (the raw console output), and `alarm-single.errors` (any panic or emulator errors).

**Debugging tip**: If a test fails, always read the `.output` file to see what your kernel actually printed!

## 3. Lab Modifications Guide

You should **not** modify the test files. If you do, you are cheating the grader. Your job is to modify the kernel code in `threads/` and `userprog/` so that all the tests pass automatically when you run `make check`.
