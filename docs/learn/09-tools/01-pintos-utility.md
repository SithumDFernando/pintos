---
module_id: "pintos-utility"
title: "The pintos Script — utils/pintos"
difficulty: "Light"
prerequisites:
  - "tools-overview"
key_concepts:
  - "QEMU invocation"
  - "Command-line argument parsing"
estimated_reading_time: "5 min"
source_files:
  - "pintos/utils/pintos"
---

# The `pintos` Script — utils/pintos

## 1. Executive Summary

The `pintos` Perl script is the primary interface for running Pintos. When you type `pintos run alarm-single`, this script:

1. Parses your command-line arguments.
2. Constructs the appropriate QEMU (or Bochs) command line.
3. Creates and attaches virtual disk images.
4. Formats Pintos-specific kernel arguments (e.g., `-q`, `-f`, `-p FILE`, `-g FILE`).
5. Boots the emulator and captures serial output.
6. Prints the output to your terminal.

## 2. Common Usage Patterns

```bash
# Run a test
pintos run alarm-single

# Run and quit after completion
pintos -v -- run alarm-single

# Format the disk
pintos -f -q

# Put a file onto the disk
pintos -p tests/userprog/args-single -a args-single -- -q

# Run a user program with arguments
pintos --filesys-size=2 -p tests/userprog/echo -a echo -- -f -q run 'echo hello world'
```

## 3. Lab Modifications Guide

This file is **not modified in any lab**.
