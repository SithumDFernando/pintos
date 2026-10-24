# Pintos Learning Curriculum & Knowledge Base

> **Comprehensive deep-dive guide, line-by-line annotations, architectural diagrams, and OS lab implementation blueprints for Stanford Pintos.**

Welcome to the **Pintos Learning Knowledge Base**. This documentation library provides a complete, structured analysis of the Pintos educational operating system codebase. Every module is crafted according to a unified pedagogical structure covering executive mental models, algorithmic foundations, architectural topologies (Mermaid flow/sequence diagrams), source code anatomy, runtime execution paths, edge cases/failure modes, and lab implementation strategies.

---

## 🎯 University OS Labs Quick Navigator

For students completing **Semester 3 OS Labs** (Lab 1: Threads and Lab 2: User Programs), use this quick roadmap to jump directly to the modules containing task analysis, design trade-offs, and file modification blueprints:

### ⭐ Lab 1: Threads & Concurrency
| Milestone / Task | Key Focus & Algorithms | Essential Modules | Key Source Files |
|---|---|---|---|
| **Alarm Clock** | Eliminate busy-waiting in `timer_sleep()` via sleeping thread queue | [03-devices/01-timer.md](./03-devices/01-timer.md)<br>[02-threads/01-thread-struct-and-lifecycle.md](./02-threads/01-thread-struct-and-lifecycle.md) | `devices/timer.c`<br>`threads/thread.c`<br>`threads/thread.h` |
| **Priority Scheduling** | Strict preemptive priority scheduling for `ready_list` | [02-threads/01-thread-struct-and-lifecycle.md](./02-threads/01-thread-struct-and-lifecycle.md)<br>[07-library/01-linked-lists.md](./07-library/01-linked-lists.md) | `threads/thread.c`<br>`threads/thread.h`<br>`lib/kernel/list.c` |
| **Priority Donation** | Priority inversion solution: Single, nested, and multiple donation | [02-threads/04-synchronization.md](./02-threads/04-synchronization.md)<br>[02-threads/01-thread-struct-and-lifecycle.md](./02-threads/01-thread-struct-and-lifecycle.md) | `threads/synch.c`<br>`threads/synch.h`<br>`threads/thread.c` |
| **Multi-Level Feedback Queue (MLFQS)** | Dynamic 4.4 fixed-point priority calculation, decay, and load avg | [02-threads/01-thread-struct-and-lifecycle.md](./02-threads/01-thread-struct-and-lifecycle.md)<br>[03-devices/01-timer.md](./03-devices/01-timer.md) | `threads/thread.c`<br>`devices/timer.c` |

---

### ⭐ Lab 2: User Programs & System Calls
| Milestone / Task | Key Focus & Algorithms | Essential Modules | Key Source Files |
|---|---|---|---|
| **Argument Passing** | Command-line parsing and x86 80x86 cdecl stack frame setup | [04-userprog/01-process-lifecycle.md](./04-userprog/01-process-lifecycle.md)<br>[07-library/07-string.md](./07-library/07-string.md) | `userprog/process.c`<br>`lib/string.c` |
| **User Memory Access** | Safe pointer verification without crashing kernel (`is_user_vaddr`, page fault) | [04-userprog/02-system-calls.md](./04-userprog/02-system-calls.md)<br>[04-userprog/03-exception-handling.md](./04-userprog/03-exception-handling.md)<br>[02-threads/06-virtual-addresses.md](./02-threads/06-virtual-addresses.md) | `userprog/syscall.c`<br>`userprog/exception.c`<br>`threads/vaddr.h` |
| **System Calls** | `halt`, `exit`, `exec`, `wait`, `create`, `remove`, `open`, `filesize`, `read`, `write`, `seek`, `tell`, `close` | [04-userprog/02-system-calls.md](./04-userprog/02-system-calls.md)<br>[07-library/12-syscall-numbers.md](./07-library/12-syscall-numbers.md)<br>[06-filesystem/01-filesys-core.md](./06-filesystem/01-filesys-core.md) | `userprog/syscall.c`<br>`userprog/syscall.h`<br>`lib/syscall-nr.h` |
| **Process Control & Synchronization** | Parent-child synchronization, exit status collection, orphan cleanup | [04-userprog/01-process-lifecycle.md](./04-userprog/01-process-lifecycle.md)<br>[02-threads/01-thread-struct-and-lifecycle.md](./02-threads/01-thread-struct-and-lifecycle.md) | `userprog/process.c`<br>`threads/thread.c` |
| **File Deny Write** | Prevent running executable binaries from being overwritten | [04-userprog/01-process-lifecycle.md](./04-userprog/01-process-lifecycle.md)<br>[06-filesystem/02-inodes.md](./06-filesystem/02-inodes.md)<br>[06-filesystem/03-files.md](./06-filesystem/03-files.md) | `userprog/process.c`<br>`filesys/inode.c`<br>`filesys/file.c` |

---

## 📚 Complete Curriculum Index

### [00 — Architecture & Build System](./00-overview/)
Foundational mental model of the hardware target, memory layout, and compilation infrastructure.
- [00-pintos-architecture.md](./00-overview/00-pintos-architecture.md) — System architecture, x86 protection rings, kernel vs user address spaces, and global subsystems.
- [01-build-system.md](./00-overview/01-build-system.md) — GNU Make recursive build system, compiler flags, target configurations, and simulator integration.

### [01 — Boot & Initialization](./01-boot-and-init/)
From raw BIOS reset vector through protected mode transition to the C runtime startup.
- [00-boot-sequence-overview.md](./01-boot-and-init/00-boot-sequence-overview.md) — End-to-end boot timeline from power-on to user prompt.
- [01-loader.md](./01-boot-and-init/01-loader.md) — 512-byte MBR assembly bootloader (`loader.S`), INT 13h disk reads, and A20 line enablement.
- [02-start.md](./01-boot-and-init/02-start.md) — 32-bit entry point (`start.S`), preliminary page directory, and segmentation setup.
- [03-kernel-linker-script.md](./01-boot-and-init/03-kernel-linker-script.md) — ELF section placement, VMA vs LMA mappings at `0xc0000000` (`kernel.lds.S`).
- [04-init.md](./01-boot-and-init/04-init.md) ⭐ *P0* — Main C entry point (`main()`), subsystem initialization sequence, and command-line option parsing.

### [02 — Threads & Synchronization](./02-threads/)
Core multi-threading kernel, context switching, interrupt mechanics, and synchronization primitives.
- [00-threading-overview.md](./02-threads/00-threading-overview.md) — Kernel threading model, thread states, and scheduling theory.
- [01-thread-struct-and-lifecycle.md](./02-threads/01-thread-struct-and-lifecycle.md) ⭐ *P1 + P2* — `struct thread` anatomy, state transitions, round-robin scheduler, and lab extension points.
- [02-context-switching.md](./02-threads/02-context-switching.md) — Assembly context switch (`switch.S`), stack frames, and register preservation.
- [03-interrupts.md](./02-threads/03-interrupts.md) — IDT setup, PIC programming, hardware IRQs, software exceptions, and `intr_frame`.
- [04-synchronization.md](./02-threads/04-synchronization.md) ⭐ *P1* — Semaphores, locks, condition variables, barriers, and priority donation algorithms.
- [05-memory-allocation.md](./02-threads/05-memory-allocation.md) — Page allocator (`palloc.c`) with user/kernel pools and kernel heap malloc (`malloc.c`).
- [06-virtual-addresses.md](./02-threads/06-virtual-addresses.md) — Address macros (`vaddr.h`), `PHYS_BASE`, page offsets, and address validation routines.
- [07-io-ports.md](./02-threads/07-io-ports.md) — x86 `inb`, `outb`, `inl`, and `outl` port I/O assembly primitives (`io.h`).
- [08-flags.md](./02-threads/08-flags.md) — CPU EFLAGS register bit definitions (`flags.h`).

### [03 — Devices & Hardware Abstractions](./03-devices/)
Hardware device drivers, timers, human interface devices, and storage controllers.
- [00-device-drivers-overview.md](./03-devices/00-device-drivers-overview.md) — Pintos device driver layer and hardware abstraction mechanisms.
- [01-timer.md](./03-devices/01-timer.md) ⭐ *P1* — Programmable Interval Timer (PIT 8254), tick handling, and sleep queue implementation.
- [02-keyboard-input.md](./03-devices/02-keyboard-input.md) — PC keyboard controller (8042), scan code translation, and input queuing.
- [03-serial-port.md](./03-devices/03-serial-port.md) — UART 16550 serial port driver, polled vs interrupt-driven console output.
- [04-interrupt-queue.md](./03-devices/04-interrupt-queue.md) — Circular FIFO buffer synchronization for device drivers (`intq.c`).
- [05-vga-display.md](./03-devices/05-vga-display.md) — VGA text mode frame buffer, cursor positioning, and screen scrolling.
- [06-block-devices.md](./03-devices/06-block-devices.md) — Abstract sector-based block device interface (`block.c`).
- [07-ide-disk.md](./03-devices/07-ide-disk.md) — ATA/IDE hard drive controller driver, PIO sector read/write transfers.
- [08-partitions.md](./03-devices/08-partitions.md) — Master Boot Record (MBR) partition table scanning and virtual partition block devices.
- [09-rtc.md](./03-devices/09-rtc.md) — Motorola 146818 Real-Time Clock driver for reading system epoch time.
- [10-shutdown.md](./03-devices/10-shutdown.md) — ACPI and APM machine power-off, reboot, and QEMU/Bochs exit sequences.
- [11-speaker.md](./03-devices/11-speaker.md) — PC speaker hardware control using PIT Channel 2.

### [04 — User Programs & Process Subsystem](./04-userprog/)
Protected user mode execution, process isolation, system calls, and x86 segmentation.
- [00-user-programs-overview.md](./04-userprog/00-user-programs-overview.md) — User process architecture, memory isolation, and kernel transition.
- [01-process-lifecycle.md](./04-userprog/01-process-lifecycle.md) ⭐ *P2* — `process_execute()`, ELF loading, cdecl argument passing stack setup, and `process_wait()`.
- [02-system-calls.md](./04-userprog/02-system-calls.md) ⭐ *P2* — `INT 0x30` software interrupt, argument extraction, pointer sanitization, and I/O syscall handlers.
- [03-exception-handling.md](./04-userprog/03-exception-handling.md) ⭐ *P2* — CPU fault dispatching, user page fault detection, and graceful process termination.
- [04-tss-and-gdt.md](./04-userprog/04-tss-and-gdt.md) — Global Descriptor Table (GDT), ring 0/3 segment descriptors, and Task State Segment (TSS) ring 0 stack pointer.
- [05-page-directories.md](./04-userprog/05-page-directories.md) — x86 two-level page table management (`pagedir.c`), CR3 register manipulation, and virtual-to-physical address translation.

### [05 — Virtual Memory](./05-virtual-memory/)
Paging concepts and foundation for demand paging, supplemental page tables, and memory-mapped files.
- [00-virtual-memory-overview.md](./05-virtual-memory/00-virtual-memory-overview.md) — Theory of demand paging, supplemental page tables, frame tables, swap tables, and eviction policies.

### [06 — File System](./06-filesystem/)
Sector-based storage layer, directory structures, and persistent file handles.
- [00-filesystem-overview.md](./06-filesystem/00-filesystem-overview.md) — High-level architecture of Pintos baseline contiguous-allocation file system.
- [01-filesys-core.md](./06-filesystem/01-filesys-core.md) — File system facade, disk formatting, and global file locks (`filesys.c`).
- [02-inodes.md](./06-filesystem/02-inodes.md) — On-disk vs in-memory inodes, contiguous sector mapping, and executable deny-write counters.
- [03-files.md](./06-filesystem/03-files.md) — Open file representations, read/write byte cursors, and seek/tell operations.
- [04-directories.md](./06-filesystem/04-directories.md) — Directory entry table, filename-to-inode lookups, and linear directory scans.
- [05-free-map.md](./06-filesystem/05-free-map.md) — Persistent disk sector bitmap allocation and block reclamation.
- [06-fsutil.md](./06-filesystem/06-fsutil.md) — Built-in kernel shell commands: `ls`, `cat`, `rm`, and `extract` (ustar tarball extractor).
- [07-off-t.md](./06-filesystem/07-off-t.md) — File offset type definition (`off_t`) and signed 32-bit boundary constraints.

### [07 — Kernel & User Standard Library](./07-library/)
Data structures, freestanding C utilities, standard formatting, and user-space wrappers.
- [00-utilities-overview.md](./07-library/00-utilities-overview.md) — Freestanding C runtime environment in kernel space vs user space.
- [01-linked-lists.md](./07-library/01-linked-lists.md) — Linux-style intrusive circular doubly linked list implementation (`list.c`).
- [02-bitmaps.md](./07-library/02-bitmaps.md) — Memory and sector tracking bitmap operations (`bitmap.c`).
- [03-hash-tables.md](./07-library/03-hash-tables.md) — Chained hash table implementation with dynamic bucket growth (`hash.c`).
- [04-debug.md](./07-library/04-debug.md) — Assertion macros (`ASSERT`), `PANIC()`, stack frame decoding, and backtraces.
- [05-stdio.md](./07-library/05-stdio.md) — Formatted printing (`printf`, `snprintf`, `vprintf`) and memory inspection hex dumps.
- [06-stdlib.md](./07-library/06-stdlib.md) — String conversion (`atoi`, `strtol`) and standard algorithms (`qsort`, `bsearch`).
- [07-string.md](./07-library/07-string.md) — Memory manipulation (`memcpy`, `memmove`, `memset`) and reentrant string tokenization (`strtok_r`).
- [08-arithmetic.md](./07-library/08-arithmetic.md) — Integer division, ceiling, and alignment rounding helper macros.
- [09-random.md](./07-library/09-random.md) — Linear congruential pseudo-random number generator.
- [10-ustar.md](./07-library/10-ustar.md) — POSIX ustar tar archive header parser.
- [11-type-headers.md](./07-library/11-type-headers.md) — Freestanding C headers (`stdbool.h`, `stddef.h`, `stdint.h`, `stdarg.h`, `inttypes.h`).
- [12-syscall-numbers.md](./07-library/12-syscall-numbers.md) — Global enumeration of system call identifiers (`SYS_*`).
- [13-user-library.md](./07-library/13-user-library.md) — User-space C library stubs, assembly syscall wrappers, and program entry `_start`.
- [14-test-infrastructure.md](./07-library/14-test-infrastructure.md) — Automated test reporting functions (`msg()`, `fail()`, `check_file()`).

### [08 — Pintos Test Suites](./08-tests/)
Automated regression test suites across kernel and user space.
- [00-testing-overview.md](./08-tests/00-testing-overview.md) — Test execution framework, Perl drivers, and output comparison mechanics.
- [01-thread-tests.md](./08-tests/01-thread-tests.md) — Project 1 test suite: alarm clock, priority donation, priority queues, and MLFQS.
- [02-userprog-tests.md](./08-tests/02-userprog-tests.md) — Project 2 test suite: argument parsing, pointer boundary validation, and syscall edge cases.
- [03-vm-tests.md](./08-tests/03-vm-tests.md) — Virtual memory demand paging and stack growth tests.
- [04-filesys-tests.md](./08-tests/04-filesys-tests.md) — File system persistence and directory tests.
- [05-internal-tests.md](./08-tests/05-internal-tests.md) — Kernel-internal unit tests for data structures (`list`, `stdio`).

### [09 — Development Tools & Sample Programs](./09-tools/)
Host-side emulation scripts, debuggers, disk generators, and user programs.
- [00-tools-overview.md](./09-tools/00-tools-overview.md) — Pintos toolchain ecosystem and development workflow.
- [01-pintos-utility.md](./09-tools/01-pintos-utility.md) — The `pintos` Perl runner script, QEMU/Bochs flags, and disk argument composition.
- [02-helper-tools.md](./09-tools/02-helper-tools.md) — `backtrace` decoder, `pintos-gdb` wrapper, and `pintos-mkdisk` utility.
- [03-example-programs.md](./09-tools/03-example-programs.md) — Reference user programs in `src/examples/` (`echo`, `cat`, `shell`, `insult`, `lineup`).

---

## 💡 Recommended Study Paths

### Path 1: Tackling Lab 1 (Threads & Scheduling)
1. Read [00-pintos-architecture.md](./00-overview/00-pintos-architecture.md) to understand kernel vs user isolation.
2. Study [02-threads/00-threading-overview.md](./02-threads/00-threading-overview.md) and [02-threads/01-thread-struct-and-lifecycle.md](./02-threads/01-thread-struct-and-lifecycle.md).
3. Dive into [07-library/01-linked-lists.md](./07-library/01-linked-lists.md) — Pintos relies extensively on intrusive doubly-linked lists for all thread scheduling queues.
4. Review [03-devices/01-timer.md](./03-devices/01-timer.md) for the Alarm Clock implementation.
5. Study [02-threads/04-synchronization.md](./02-threads/04-synchronization.md) for Priority Donation and Condition Variables.
6. Verify your solutions against [08-tests/01-thread-tests.md](./08-tests/01-thread-tests.md).

### Path 2: Tackling Lab 2 (User Programs & Syscalls)
1. Understand protected mode transition and ring 0/3 segmentation in [04-userprog/04-tss-and-gdt.md](./04-userprog/04-tss-and-gdt.md).
2. Trace the executable ELF loading and stack setup in [04-userprog/01-process-lifecycle.md](./04-userprog/01-process-lifecycle.md).
3. Learn argument string parsing with [07-library/07-string.md](./07-library/07-string.md) (`strtok_r`).
4. Read [02-threads/06-virtual-addresses.md](./02-threads/06-virtual-addresses.md) and [04-userprog/03-exception-handling.md](./04-userprog/03-exception-handling.md) to master user pointer safety checks.
5. Implement syscall dispatching using [04-userprog/02-system-calls.md](./04-userprog/02-system-calls.md) and [07-library/12-syscall-numbers.md](./07-library/12-syscall-numbers.md).
6. Connect user file system calls to [06-filesystem/01-filesys-core.md](./06-filesystem/01-filesys-core.md), [06-filesystem/02-inodes.md](./06-filesystem/02-inodes.md), and [06-filesystem/03-files.md](./06-filesystem/03-files.md).
7. Validate passing requirements with [08-tests/02-userprog-tests.md](./08-tests/02-userprog-tests.md).
