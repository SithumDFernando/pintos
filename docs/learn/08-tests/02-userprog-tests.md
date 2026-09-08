---
module_id: "userprog-tests"
title: "User Program Tests — tests/userprog/"
difficulty: "Light"
prerequisites:
  - "testing-overview"
key_concepts:
  - "Argument passing tests"
  - "System call tests"
  - "Robustness tests"
estimated_reading_time: "5 min"
source_files:
  - "pintos/tests/userprog/"
---

# User Program Tests — tests/userprog/

## 1. Executive Summary

The `tests/userprog/` directory contains all test programs for **Project 2 (User Programs)**. They are grouped into categories:

### Argument Passing
| Test | What It Checks |
|---|---|
| `args-none` | Running a program with no arguments. |
| `args-single` | A single argument is passed correctly. |
| `args-multiple` | Multiple arguments are passed and in correct order. |
| `args-many` | Many arguments (stress test). |
| `args-dbl-space` | Double spaces between arguments are handled. |

### System Call Tests
| Test | What It Checks |
|---|---|
| `halt` | `SYS_HALT` powers off the machine. |
| `exit` | `SYS_EXIT` returns the correct status code. |
| `create-*` | File creation (normal, empty, long name, already exists). |
| `open-*` | File opening (normal, missing, twice). |
| `read-*` | Reading from files and stdin. |
| `write-*` | Writing to files and stdout. |
| `close-*` | Closing files (normal, twice, stdin, stdout). |
| `exec-*` | Executing child processes. |
| `wait-*` | Waiting for child processes. |
| `multi-recurse` | Recursive process execution. |

### Robustness / Bad Pointer Tests
| Test | What It Checks |
|---|---|
| `sc-bad-sp` | System call with bad stack pointer. |
| `sc-bad-arg` | System call arguments cross into kernel space. |
| `sc-boundary-*` | Arguments span page boundaries. |
| `bad-read`, `bad-write`, `bad-jump` | Dereferencing NULL, writing to kernel, jumping to kernel. |

## 2. Lab Modifications Guide

Do **not** modify test files. Your kernel changes must make these tests pass.
