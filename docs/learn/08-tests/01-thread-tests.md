---
module_id: "thread-tests"
title: "Thread Tests — tests/threads/"
difficulty: "Light"
prerequisites:
  - "testing-overview"
key_concepts:
  - "Alarm clock tests"
  - "Priority scheduling tests"
  - "MLFQS tests"
estimated_reading_time: "5 min"
source_files:
  - "pintos/tests/threads/"
---

# Thread Tests — tests/threads/

## 1. Executive Summary

The `tests/threads/` directory contains all test programs for **Project 1 (Threads)**. They are grouped into three categories:

### Alarm Clock Tests
| Test | What It Checks |
|---|---|
| `alarm-single` | A single thread sleeps and wakes up on time. |
| `alarm-multiple` | Multiple threads sleep for different durations. |
| `alarm-simultaneous` | Multiple threads sleep for the same duration. |
| `alarm-priority` | Higher-priority threads wake up first. |
| `alarm-zero` | Sleep for 0 ticks (should return immediately). |
| `alarm-negative` | Sleep for negative ticks (should return immediately). |

### Priority Scheduling Tests
| Test | What It Checks |
|---|---|
| `priority-change` | Changing a thread's priority causes immediate preemption. |
| `priority-donate-one` | Basic priority donation to a lock holder. |
| `priority-donate-multiple` | A thread receives donations from multiple donors. |
| `priority-donate-multiple2` | Multiple locks, multiple donors. |
| `priority-donate-nest` | Nested donation (A waits on B waits on C). |
| `priority-donate-sema` | Priority donation interacts with semaphores. |
| `priority-donate-lower` | Lowering priority below donated priority has no effect. |
| `priority-fifo` | Equal-priority threads run in FIFO order. |
| `priority-preempt` | Creating a higher-priority thread preempts the current thread. |
| `priority-sema` | `sema_up` wakes the highest-priority waiter. |
| `priority-condvar` | `cond_signal` wakes the highest-priority waiter. |

### MLFQS Tests
| Test | What It Checks |
|---|---|
| `mlfqs-load-1`, `mlfqs-load-60`, `mlfqs-load-avg` | `load_avg` calculation correctness. |
| `mlfqs-recent-1` | `recent_cpu` calculation correctness. |
| `mlfqs-fair-*` | Fair CPU sharing among threads with different `nice` values. |
| `mlfqs-block` | Blocked threads don't accumulate `recent_cpu`. |

## 2. Lab Modifications Guide

Do **not** modify test files. Your kernel changes must make these tests pass.
