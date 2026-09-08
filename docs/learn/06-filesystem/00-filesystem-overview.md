---
module_id: "filesystem-overview"
title: "File System — filesys/ (Project 4)"
difficulty: "Advanced"
prerequisites:
  - "block-devices"
key_concepts:
  - "Inodes"
  - "Buffer Cache"
  - "Indexed Allocation"
estimated_reading_time: "5 min"
source_files:
  - "pintos/src/filesys/"
---

# File System — filesys/ (Project 4)

## 1. Executive Summary & Mental Model

The `filesys/` directory contains the Pintos file system. It provides the abstraction of "Files" and "Directories" on top of the raw "Sectors" provided by `block.c`.

The base file system provided with Pintos works, but it is extremely primitive:
- Files are allocated as a single, contiguous chunk of disk sectors. (If you want a 100 KB file, you must find 100 KB of contiguous free space).
- Files cannot grow once created.
- There are no subdirectories (only a single root directory).

## 2. Key Files in `filesys/`

- `filesys.c`: The top-level interface for formatting the disk and creating/opening files.
- `file.c`: Represents an open file. Tracks the current read/write position (`off_t`).
- `inode.c`: The "Index Node". This is the metadata for a file on disk (its size, where its sectors are). In the base OS, an inode points to a contiguous array of sectors.
- `directory.c`: A special type of file that contains a list of file names and their corresponding inode numbers.
- `free_map.c`: A bitmap stored on disk that tracks which sectors are currently in use.

## 3. Lab Modifications Guide

Since your module only requires completing up to **Lab 2**, you will use the file system (via `filesys_open`, `file_read`, etc.) inside your System Call handler, but you will not modify its internal implementation.

*(If you were doing Project 4, you would rewrite `inode.c` to use indexed allocation so files could grow, add subdirectory support to `directory.c`, and implement a Buffer Cache to dramatically speed up reads and writes).*
