---
module_id: "speaker"
title: "PC Speaker — speaker.c"
difficulty: "Light"
prerequisites:
  - "io-ports"
key_concepts:
  - "PIT Channel 2"
  - "PC Speaker toggling"
estimated_reading_time: "2 min"
source_files:
  - "pintos/src/devices/speaker.c"
---

# PC Speaker — speaker.c

## 1. Executive Summary & Mental Model

This module controls the classic internal PC speaker, allowing the OS to make beeping noises. It is rarely used in normal Pintos operation but exists for completeness.

## 2. Source Code Anatomy & Deep Dive

The PC Speaker is wired directly to **Channel 2** of the 8254 PIT (the same timer chip used for the system clock on Channel 0).

- **`speaker_on(frequency)`**: Programs PIT Channel 2 to oscillate at the given frequency, and enables the speaker bit via the keyboard controller port `0x61`.
- **`speaker_off()`**: Disables the speaker bit.
- **`speaker_beep()`**: Turns the speaker on, calls `timer_msleep(250)`, and turns it off.

## 3. Lab Modifications Guide

This file is **not modified in any lab**.
