---
module_id: "exception-handling"
title: "Exception Handling — exception.c"
difficulty: "Intermediate"
prerequisites:
  - "system-calls"
key_concepts:
  - "Hardware Exceptions"
  - "Page Faults"
  - "Kernel vs User Faults"
estimated_reading_time: "15 min"
source_files:
  - "pintos/src/userprog/exception.h"
  - "pintos/src/userprog/exception.c"
---

# Exception Handling — exception.c ⭐ P2

## 1. Executive Summary & Mental Model

While system calls are *intentional* traps into the kernel, **exceptions** are *accidental* traps. They occur when a program does something illegal, like dividing by zero, executing an invalid opcode, or accessing memory it doesn't own.

**Real-world analogy**: 
If a System Call is handing a withdrawal slip to a bank teller, an Exception is climbing over the counter to grab the money yourself. The bank's security system (the CPU hardware) instantly freezes you and calls the police (the Exception Handler).

## 2. Theoretical Foundations

### x86 Exception Vectors

The first 32 interrupt vectors (0x00 to 0x1F) are reserved by Intel for CPU exceptions.
Some common ones:
- **Vector 0**: Divide by Zero (`#DE`)
- **Vector 13**: General Protection Fault (`#GP`) - Usually caused by privilege violations.
- **Vector 14**: Page Fault (`#PF`) - Accessing an unmapped or protected virtual address.

## 3. Source Code Anatomy & Deep Dive

### `exception_init()`

This function registers handlers for all the Intel exceptions. 
Notice the `DPL` (Descriptor Privilege Level) parameter in `intr_register_int`:
- Most exceptions are registered with DPL 0. This means user programs (Ring 3) cannot intentionally trigger them using the `int` instruction. If they try, they get a General Protection Fault.
- Breakpoints (Vector 3) are registered with DPL 3 so debuggers can use them.

### `kill()`

When a generic exception occurs, it routes to `kill()`.
- If the exception happened in user code (`f->cs == SEL_UCSEG`), the kernel prints a message and terminates the user thread (`thread_exit()`).
- If the exception happened in kernel code (`f->cs == SEL_KCSEG`), it means there is a bug in your OS! The kernel calls `PANIC` and halts the entire system.

### `page_fault()`

Page Faults (Vector 14) are special. They have their own handler because they are the foundation of Virtual Memory.
When a page fault occurs:
1. The CPU places the memory address that caused the fault into a special register called `CR2`.
2. The handler reads `CR2` into `fault_addr`.
3. The handler checks `f->error_code` to determine *why* the fault happened:
   - `PF_P`: Was the page not present (0), or was it a rights violation (1)?
   - `PF_W`: Was it a read (0) or a write (1)?
   - `PF_U`: Did it happen in kernel mode (0) or user mode (1)?

Currently, `page_fault()` just prints a message and calls `kill()`.

## 4. Lab Modifications Guide

### ⭐ P2: Safe Memory Access (Page Fault Catching)

As mentioned in the System Calls module, one way to validate user pointers is to simply dereference them and catch the resulting Page Fault.

If you choose this method, you must modify `page_fault()`:
1. If the fault happened in **user mode** (`user == true`), simply call `thread_exit()` (or explicitly exit with status -1).
2. If the fault happened in **kernel mode** (`user == false`), it gets tricky. Normally, a kernel page fault is a bug (a PANIC). BUT, if the kernel was intentionally dereferencing a user-provided pointer during a system call, we want to catch it gracefully.
   - **Action**: You must set a flag in `struct thread` (e.g., `t->in_syscall = true`) when entering a syscall. 
   - If a kernel page fault occurs while `t->in_syscall` is true, do not PANIC. Instead, modify the `f->eip` (the instruction pointer) to jump to a safe recovery routine, or simply set the thread's exit code to -1 and call `thread_exit()`. (Modifying `f->eip` and `f->eax` is the professional way, utilizing a technique often called `copy_from_user` in Linux).
