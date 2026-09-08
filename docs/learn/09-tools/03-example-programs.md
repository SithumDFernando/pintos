---
module_id: "example-programs"
title: "Example Programs — examples/"
difficulty: "Light"
prerequisites:
  - "user-programs-overview"
key_concepts:
  - "Sample user programs"
  - "System call usage"
estimated_reading_time: "3 min"
source_files:
  - "pintos/src/examples/"
---

# Example Programs — examples/

## 1. Executive Summary

The `examples/` directory contains simple user programs that demonstrate how to use Pintos system calls. These are useful for manual testing and understanding the expected behavior of your syscall implementations.

| Program | Purpose |
|---|---|
| `halt` | Calls `halt()` to shut down the machine. Simplest possible test. |
| `echo` | Prints its command-line arguments. Tests argument passing. |
| `cat` | Reads a file and prints its contents to stdout. Tests `open`, `read`, `write`, `close`. |
| `cp` | Copies one file to another. Tests `create`, `open`, `read`, `write`, `close`. |
| `hex-dump` | Hex-dumps a file. |
| `ls` | Lists files in the root directory (requires `readdir` syscall from P4). |
| `mkdir` | Creates a directory (requires P4). |
| `rm` | Deletes a file. Tests `remove`. |
| `shell` | A basic interactive shell. Tests `exec`, `wait`, `read` (stdin), `write` (stdout). |
| `insult` | Prints a random insult. For fun. |
| `lineup` | Demonstrates inter-process communication. |
| `matmult` | Matrix multiplication (stress test for VM). |
| `mcat`, `mcp` | Memory-mapped versions of cat/cp (requires P3). |
| `bubsort` | Bubble sort (stress test). |

## 2. Lab Modifications Guide

These programs are **not modified in any lab**. They are provided as-is for testing your system call implementations. Use them with:
```bash
pintos -p examples/echo -a echo -- -f -q run 'echo hello world'
```
