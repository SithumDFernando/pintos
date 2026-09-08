---
module_id: "partitions"
title: "Disk Partitions — partition.c"
difficulty: "Light"
prerequisites:
  - "block-devices"
key_concepts:
  - "Master Boot Record (MBR)"
  - "Partition tables"
  - "Block device wrapping"
estimated_reading_time: "4 min"
source_files:
  - "pintos/src/devices/partition.c"
---

# Disk Partitions — partition.c

## 1. Executive Summary & Mental Model

A physical hard drive can be divided into smaller logical drives called **partitions**. The `partition.c` file reads the partition table on a raw disk and creates a new "fake" block device for each partition.

**Real-world analogy**: 
The IDE driver provides a massive apartment building (the raw disk). The partition driver draws lines down the middle of the building, creating completely independent wings (partitions), and gives the File System the keys to just one wing. The File System doesn't even know the other wings exist.

## 2. Source Code Anatomy & Deep Dive

When Pintos boots (if `FILESYS` is enabled), `partition_scan()` reads the very first sector (Sector 0) of every IDE disk. This is the **Master Boot Record (MBR)**.

The MBR contains a table listing up to 4 partitions, including their starting sector and length.

For each valid partition found, the driver creates a new `struct block`. When the file system calls `block_read(sector 5)` on the partition, the partition driver translates that to `block_read(sector 5 + start_offset)` and forwards the request to the underlying IDE driver.

## 3. Lab Modifications Guide

This file is **not modified in any lab**.
