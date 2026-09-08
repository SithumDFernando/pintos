---
module_id: "free-map"
title: "Free Map — free-map.c & free-map.h"
difficulty: "Moderate"
prerequisites:
  - "bitmaps"
  - "inodes"
key_concepts:
  - "Disk sector allocation"
  - "Persistent bitmap"
estimated_reading_time: "5 min"
source_files:
  - "pintos/src/filesys/free-map.c"
  - "pintos/src/filesys/free-map.h"
---

# Free Map — free-map.c & free-map.h

## 1. Executive Summary & Mental Model

The free map is a **bitmap stored on disk** (at sector 0, `FREE_MAP_SECTOR`) that tracks which disk sectors are currently in use. It is the disk-level equivalent of `palloc`'s bitmap for RAM pages.

## 2. Source Code Anatomy & Deep Dive

- `free_map_allocate(cnt, &sector)`: Finds `cnt` contiguous free sectors using `bitmap_scan_and_flip()`. Writes the updated bitmap to disk immediately.
- `free_map_release(sector, cnt)`: Marks `cnt` sectors starting at `sector` as free.
- `free_map_open/close()`: Reads/writes the bitmap from/to a file on disk.
- `free_map_create()`: Creates the initial free-map file during formatting.

Note that sectors 0 (`FREE_MAP_SECTOR`) and 1 (`ROOT_DIR_SECTOR`) are always marked as used during `free_map_init()`.

## 3. Lab Modifications Guide

This file is **not modified in any lab**.
