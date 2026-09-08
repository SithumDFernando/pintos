---
module_id: "filesys-tests"
title: "File System Tests — tests/filesys/"
difficulty: "Light"
prerequisites:
  - "testing-overview"
key_concepts:
  - "Base file system tests"
  - "Extended file system tests"
estimated_reading_time: "2 min"
source_files:
  - "pintos/tests/filesys/"
---

# File System Tests — tests/filesys/

## 1. Executive Summary

The `tests/filesys/` directory contains tests for the file system, split into:
- **`base/`**: Tests for the base (unmodified) file system. These should pass without any changes if you have the user program infrastructure working in P2.
- **`extended/`**: Tests for Project 4 (File System Extensions) including file growth, subdirectories, and buffer caching.

## 2. Lab Modifications Guide

Not applicable — File system modifications are out of scope for your module.
