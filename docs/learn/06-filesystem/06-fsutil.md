---
module_id: "fsutil"
title: "File System Utilities — fsutil.c"
difficulty: "Moderate"
prerequisites:
  - "filesys-core"
key_concepts:
  - "ustar archive extraction"
  - "Kernel-level file commands"
estimated_reading_time: "5 min"
source_files:
  - "pintos/src/filesys/fsutil.c"
---

# File System Utilities — fsutil.c

## 1. Executive Summary & Mental Model

`fsutil.c` provides kernel-level command-line actions for manipulating the file system during boot, before any user programs run. These are triggered by command-line arguments to the `pintos` script (e.g., `-ls`, `-cat`, `-p`, `-g`).

## 2. Source Code Anatomy & Deep Dive

| Function | Pintos CLI | Purpose |
|---|---|---|
| `fsutil_ls()` | `-ls` | Lists files in the root directory. |
| `fsutil_cat(argv)` | `-cat FILE` | Hex-dumps a file to the console. |
| `fsutil_rm(argv)` | `-rm FILE` | Deletes a file from the file system. |
| `fsutil_extract()` | `-p FILE` (put) | Extracts a ustar-format tar archive from the "scratch" block device into the file system. This is how test programs get copied onto the virtual disk. |
| `fsutil_append(argv)` | `-g FILE` (get) | Copies a file from the Pintos file system back to the scratch device in ustar format (to retrieve it on the host). |

The `extract` and `append` functions use the **BLOCK_SCRATCH** device (a second virtual disk) as a staging area. The `pintos` script copies files between the host filesystem and the scratch disk using `dd`.

## 3. Lab Modifications Guide

This file is **not modified in any lab**.
