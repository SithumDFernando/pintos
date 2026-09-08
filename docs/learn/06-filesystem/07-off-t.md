---
module_id: "off-t"
title: "File Offset Type — off_t.h"
difficulty: "Light"
prerequisites: []
key_concepts:
  - "Type alias"
estimated_reading_time: "1 min"
source_files:
  - "pintos/src/filesys/off_t.h"
---

# File Offset Type — off_t.h

## 1. Executive Summary

`off_t.h` defines the type alias `off_t` as `int32_t`. This is the type used for file offsets and sizes throughout the file system. It limits individual files to 2 GB, which is more than sufficient for Pintos.

```c
typedef int32_t off_t;
#define PROTd PRId32   /* printf format specifier for off_t */
```

## 2. Lab Modifications Guide

This file is **not modified in any lab**.
