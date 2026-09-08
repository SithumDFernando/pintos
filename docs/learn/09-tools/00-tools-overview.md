---
module_id: "tools-overview"
title: "Tools & Utilities — utils/"
difficulty: "Beginner"
prerequisites:
  - "build-system"
key_concepts:
  - "pintos script"
  - "pintos-mkdisk"
  - "GDB macros"
estimated_reading_time: "5 min"
source_files:
  - "pintos/utils/"
---

# Tools & Utilities — utils/

## 1. Executive Summary & Mental Model

The `utils/` directory contains host-side scripts (mostly Perl and Python) that help you interact with the Pintos OS from your host development machine (Linux/Ubuntu). 

These are not part of the OS itself; they are the tools you use to build it, run it, and debug it.

## 2. Key Tools

- **`pintos`**: The main wrapper script. When you type `pintos run alarm-single`, this script figures out how to invoke QEMU or Bochs, attaches the virtual hard drives, formats the command line arguments, and pipes the output back to your terminal.
- **`pintos-mkdisk`**: A script that creates an empty file on your host machine and formats it as a valid Pintos virtual hard drive.
- **`pintos-gdb`**: A wrapper around the GNU Debugger (GDB) configured specifically for Pintos, making it easier to attach to a running emulator.
- **`pintos.pm` / `Pintos.pm`**: Perl modules that contain shared logic used by the testing infrastructure to parse Pintos output.
- **`Makefile`**: The host-side makefile that compiles these utilities if necessary.

## 3. Lab Modifications Guide

You will **not** modify these files. However, you will use the `pintos` utility script every time you manually run your OS or execute the test suite. If you ever want to see exactly how QEMU is being invoked behind the scenes, you can read the `pintos` perl script.
