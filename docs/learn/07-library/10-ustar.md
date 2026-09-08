---
module_id: "ustar"
title: "Ustar Archive Format — ustar.c"
difficulty: "Light"
prerequisites: []
key_concepts:
  - "Tar archive format"
estimated_reading_time: "2 min"
source_files:
  - "pintos/src/lib/ustar.c"
  - "pintos/src/lib/ustar.h"
---

# Ustar Archive Format — ustar.c

## 1. Executive Summary

Provides functions for parsing and creating **ustar** (Unix Standard TAR) archive headers. This is the format used by `fsutil_extract` and `fsutil_append` to transfer files between the host OS and the Pintos virtual disk via the scratch block device.

- `ustar_parse_header()`: Reads a 512-byte header and extracts the file name, type (regular file, directory, EOF), and size.
- `ustar_make_header()`: Creates a 512-byte header for a given file name and size.

## 2. Lab Modifications Guide

This file is **not modified in any lab**.
