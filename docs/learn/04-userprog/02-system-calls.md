---
module_id: "system-calls"
title: "System Calls — syscall.c & syscall.h (Full Depth)"
difficulty: "Advanced"
prerequisites:
  - "user-programs-overview"
  - "interrupts"
key_concepts:
  - "Software Interrupt (int 0x30)"
  - "System call numbers"
  - "User memory validation"
  - "File descriptor management"
estimated_reading_time: "25 min"
source_files:
  - "pintos/src/userprog/syscall.h"
  - "pintos/src/userprog/syscall.c"
  - "pintos/lib/syscall-nr.h"
---

# System Calls — syscall.c & syscall.h ⭐ P2

## 1. Executive Summary & Mental Model

Because user programs run in an isolated environment (Ring 3), they cannot access the disk, the screen, or the keyboard directly. To do anything useful, they must ask the kernel for help. They do this by triggering a **System Call**.

**Real-world analogy**: 
A System Call is like submitting a form at a bank. You (the User Program) cannot walk into the vault (Kernel Memory) to get your money. Instead, you fill out a withdrawal slip (Arguments on the Stack), hand it to the teller via the glass window (Software Interrupt `int 0x30`), and wait for the teller (Syscall Handler) to retrieve the money and hand it back to you.

**Primary CS problems solved**:
- Privilege separation.
- Safe cross-boundary data transfer.
- Resource tracking (File descriptors).

## 2. Theoretical Foundations

### The Software Interrupt (`int 0x30`)

Unlike hardware interrupts (which are unpredictable and triggered by the timer or keyboard), a software interrupt is triggered intentionally by the program using the x86 `int` instruction.

In Pintos, user programs execute `int 0x30` to trigger a system call. 
Before executing the instruction, the user program's C library pushes the arguments onto its stack, and pushes the **System Call Number** (e.g., `SYS_EXIT` is 1, `SYS_WRITE` is 9) right on top.

When `int 0x30` executes, the CPU:
1. Switches to Kernel Mode (Ring 0).
2. Switches to the kernel stack for the current thread.
3. Saves the user's stack pointer in the `intr_frame` (`f->esp`).
4. Jumps to `syscall_handler()`.

## 3. Source Code Anatomy & Deep Dive

### Current Implementation

```c
void syscall_init (void) {
  // Registers vector 0x30 with DPL 3 (User mode allowed)
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void syscall_handler (struct intr_frame *f UNUSED) {
  printf ("system call!\n");
  thread_exit ();
}
```
Right now, `syscall_handler` just prints a message and kills the thread. In Project 2, this function becomes the main router for all OS services.

### Reading Arguments from the User Stack

Inside `syscall_handler`, `f->esp` points to the user's stack. 
If the user called `write(fd, buffer, size)`, the stack looks like this (from bottom to top):
- `f->esp + 12`: `size`
- `f->esp + 8`: `buffer` pointer
- `f->esp + 4`: `fd`
- `f->esp + 0`: `SYS_WRITE` (System call number)

You must read `*(int*)(f->esp)` to get the syscall number, and then use a `switch` statement to call the appropriate kernel function.

## 4. Edge Cases, Failure Modes & Resilience

The kernel must be extremely paranoid about data provided by the user. 
If a malicious or buggy user program passes a bad pointer (e.g., pointing to kernel memory, or unmapped memory), and the kernel blindly dereferences it, the **entire OS will crash** with a Page Fault in kernel mode.

| Attack / Bug | Kernel Defense Requirement |
|---|---|
| User passes pointer `0xC0001000` (Kernel memory) | You must check `is_user_vaddr(ptr)`. If false, kill the process. |
| User passes pointer to unmapped memory | You must verify the page exists via `pagedir_get_page()`. |
| User passes a string, but it's not null-terminated | You must check every single byte up to the null terminator to ensure it stays within valid user memory. |
| User tries to close `fd` 9999 | Maintain a valid File Descriptor table. Reject invalid FDs. |

## 5. Lab Modifications Guide

### ⭐ P2: Safe Memory Access

Before reading any argument from `f->esp`, or accessing any buffer/string passed by the user, you must validate the pointer.
There are two ways to do this (see Pintos manual):
1. **Verify then Access**: Use `is_user_vaddr(ptr)` AND `pagedir_get_page(thread_current()->pagedir, ptr)`. If it returns NULL, kill the process with exit code -1. (Easier, but slower).
2. **Access then Catch**: Rely on the MMU. Check `is_user_vaddr(ptr)`. Then try to dereference it. If it page faults, modify `page_fault()` in `exception.c` to catch it and kill the process. (Harder, but faster and preferred in real OSes).

### ⭐ P2: Implementing System Calls

You must implement the following system calls:
- `halt`: Call `shutdown_power_off()`.
- `exit(status)`: Save status, print `"process_name: exit(status)"`, call `thread_exit()`.
- `exec(cmd)`: Call `process_execute()`. Wait for load. Return PID.
- `wait(pid)`: Call `process_wait()`.

**File System Calls**:
The Pintos file system (`filesys/`) is NOT thread-safe. You must protect all calls to `filesys_open`, `file_read`, etc., with a single global lock (e.g., `filesys_lock`).

- `create`, `remove`, `open`, `filesize`, `read`, `write`, `seek`, `tell`, `close`.

**File Descriptors**:
- When a user opens a file, the kernel returns an integer (File Descriptor).
- `STDIN` (0) and `STDOUT` (1) are reserved.
- Add an array or list of `struct file *` to `struct thread` to track open files. For example, `thread_current()->fd_table[2]` holds the `struct file *` for FD 2.
- When `write` is called on FD 1, route it to `putbuf()` (console output).
- When `read` is called on FD 0, route it to `input_getc()` (keyboard).
- When a thread exits, loop through its FD table and close all open files to prevent memory leaks!
