---
module_id: "tss-and-gdt"
title: "TSS and GDT — tss.c & gdt.c"
difficulty: "Light"
prerequisites:
  - "user-programs-overview"
key_concepts:
  - "Global Descriptor Table (GDT)"
  - "Task-State Segment (TSS)"
  - "Ring 3 to Ring 0 stack switching"
estimated_reading_time: "10 min"
source_files:
  - "pintos/src/userprog/tss.c"
  - "pintos/src/userprog/gdt.c"
---

# TSS and GDT — tss.c & gdt.c

## 1. Executive Summary & Mental Model

These two files configure the lowest-level x86 hardware structures required for User Mode to exist. They tell the CPU what "Kernel Mode" and "User Mode" actually mean, and how to safely transition between them during an interrupt.

## 2. Source Code Anatomy & Deep Dive

### The GDT (`gdt.c`)

The **Global Descriptor Table (GDT)** is an array in memory that defines memory "Segments" and their privilege levels.
Pintos creates 5 main segments:
1. Null Segment (Required by CPU)
2. **Kernel Code** (`SEL_KCSEG`): DPL 0 (Ring 0)
3. **Kernel Data** (`SEL_KDSEG`): DPL 0 (Ring 0)
4. **User Code** (`SEL_UCSEG`): DPL 3 (Ring 3)
5. **User Data** (`SEL_UDSEG`): DPL 3 (Ring 3)

When the CPU executes code, the `CS` (Code Segment) register holds an index into this table. The CPU checks the DPL of the segment. If it's 3, the CPU enforces User Mode restrictions. If it's 0, the CPU allows everything.

### The TSS (`tss.c`)

The **Task-State Segment (TSS)** is an old x86 feature originally designed for hardware multitasking. Modern OSes (including Linux and Pintos) ignore 99% of it, preferring software context switching.

**However, one feature of the TSS is absolutely mandatory**: Stack Switching on Interrupts.

When a user program is running (Ring 3), its stack pointer (`ESP`) points to `0xC0000000`. 
If a timer interrupt fires, the CPU switches to Kernel Mode (Ring 0). Where does the CPU put the interrupt frame? It cannot use the user's stack, because the user might have provided a garbage stack pointer!

The CPU looks at the **TSS**. Specifically, it looks at the `esp0` field (Ring 0 Stack Pointer). 
In Pintos, `tss_update()` is called on every context switch to update `esp0` to point to the top of the current thread's 4 KB kernel stack page.
When the interrupt fires, the CPU hardware automatically loads `ESP` with this safe kernel stack before pushing anything.

## 3. Lab Modifications Guide

These files are **not modified in any lab**. 
Understanding `tss_update()` is helpful for understanding how `process_activate()` works during context switches, but you do not need to change the code.
