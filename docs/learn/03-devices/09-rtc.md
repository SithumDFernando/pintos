---
module_id: "rtc"
title: "Real-Time Clock — rtc.c"
difficulty: "Light"
prerequisites:
  - "io-ports"
key_concepts:
  - "CMOS RAM"
  - "Epoch time"
estimated_reading_time: "3 min"
source_files:
  - "pintos/src/devices/rtc.c"
---

# Real-Time Clock — rtc.c

## 1. Executive Summary & Mental Model

The Real-Time Clock (RTC) is a battery-powered chip on the motherboard that keeps track of the current date and time, even when the computer is turned off. 

It is completely separate from the PIT (Programmable Interval Timer, `timer.c`), which only measures time *since boot*.

## 2. Source Code Anatomy & Deep Dive

The RTC is accessed by reading from CMOS memory using Port I/O (ports `0x70` and `0x71`).

`rtc_get_time()` reads the year, month, day, hour, minute, and second from the CMOS, converting them from BCD (Binary Coded Decimal) to standard binary. It then calculates the number of seconds since January 1, 1970 (the UNIX Epoch).

This value is used in `init.c` to seed the random number generator (`random_init()`).

## 3. Lab Modifications Guide

This file is **not modified in any lab**.
