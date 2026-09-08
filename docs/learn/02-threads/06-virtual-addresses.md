---
module_id: "virtual-addresses"
title: "Virtual Addresses & Page Tables — vaddr.h & pte.h"
difficulty: "Intermediate"
prerequisites:
  - "boot-start"
  - "memory-allocation"
key_concepts:
  - "Virtual vs Physical addresses"
  - "x86 Page Directory and Page Tables"
  - "Address translation macros"
  - "Page alignment"
estimated_reading_time: "15 min"
source_files:
  - "pintos/src/threads/vaddr.h"
  - "pintos/src/threads/pte.h"
---

# Virtual Addresses & Page Tables — vaddr.h & pte.h

## 1. Executive Summary & Mental Model

Modern operating systems use **Virtual Memory** to give every program the illusion that it has its own massive, contiguous chunk of memory starting at address 0. `vaddr.h` and `pte.h` contain the mathematical macros and definitions required to translate between these fake "virtual" addresses and real "physical" addresses.

**Real-world analogy**: 
- **Virtual Address**: A contact's name in your phone book ("Mom").
- **Physical Address**: The actual phone number ("555-1234").
- **Page Table**: The contacts app that translates the name to the number. 

Just as two different people might have "Mom" in their phone but it dials a completely different physical phone, two different Pintos processes can access virtual address `0x08048000` but read completely different physical RAM chips.

## 2. Theoretical Foundations

### x86 32-bit Paging Architecture

A 32-bit virtual address (like `0xC0020000`) is split into three parts by the CPU hardware:
1. **Top 10 bits (Page Directory Index)**: Which page table to use. (0-1023)
2. **Middle 10 bits (Page Table Index)**: Which physical page to use. (0-1023)
3. **Bottom 12 bits (Page Offset)**: The exact byte within the 4 KB page. (0-4095)

```
 31                  22 21                  12 11                   0
+----------------------+----------------------+----------------------+
| Page Directory Index |   Page Table Index   |    Page Offset       |
+----------------------+----------------------+----------------------+
```

### The Kernel Identity Mapping

In Pintos, the kernel's virtual memory is "identity mapped" with an offset.
Physical address `X` is always accessible to the kernel at virtual address `PHYS_BASE + X` (where `PHYS_BASE` is `0xC0000000` or 3 GB).
- Physical `0x00000000` -> Virtual `0xC0000000`
- Physical `0x00020000` -> Virtual `0xC0020000`

## 3. Architecture & Visual Topography

### Memory Space Division

```
0xFFFFFFFF +----------------------------------+
           |                                  |
           |          Kernel Space            |
           |  (Shared across all processes)   |
           |                                  |
0xC0000000 +----------------------------------+ <-- PHYS_BASE
           |                                  |
           |           User Space             |
           |  (Unique to each user process)   |
           |                                  |
0x00000000 +----------------------------------+
```

## 4. Source Code Anatomy & Deep Dive

### `vaddr.h` — Virtual Address Math

This file provides macros to dissect a virtual address without knowing anything about hardware page tables.

```c
#define PGBITS  12                         /* Number of offset bits. */
#define PGSIZE  (1 << PGBITS)              /* Bytes in a page (4096). */
#define PGMASK  BITMASK(PGSHIFT, PGBITS)   /* 0x00000FFF */
```

- **`pg_ofs(va)`**: Returns the offset within the page (the bottom 12 bits). e.g., `pg_ofs(0xC0020123) == 0x123`.
- **`pg_no(va)`**: Returns the virtual page number (shifts away the bottom 12 bits).
- **`pg_round_down(va)`**: Clears the bottom 12 bits to find the start of the page. e.g., `pg_round_down(0xC0020123) == 0xC0020000`. Used heavily to find `struct thread`.
- **`ptov(paddr)`**: Converts Physical To Virtual by adding `PHYS_BASE`.
- **`vtop(vaddr)`**: Converts Virtual To Physical by subtracting `PHYS_BASE`.

### `pte.h` — Hardware Page Tables

This file deals specifically with the x86 page table entries (PTEs) and page directory entries (PDEs).

```c
#define PTE_P 0x1               /* 1=present, 0=not present. */
#define PTE_W 0x2               /* 1=read/write, 0=read-only. */
#define PTE_U 0x4               /* 1=user/kernel, 0=kernel only. */
```

A Page Table Entry is a 32-bit integer. The top 20 bits are the physical address of the 4 KB page. The bottom 12 bits are flags (Present, Writable, User-accessible, etc.).

- **`pde_create(pt)`**: Creates a Page Directory Entry pointing to a Page Table (`pt`).
- **`pte_create_user(page, writable)`**: Creates a Page Table Entry pointing to a physical `page`. Marks it as accessible by User code (`PTE_U`), Present (`PTE_P`), and optionally writable (`PTE_W`).

## 5. Runtime Execution & Sequence

When a user program executes `movl %eax, 0x08048000`:
1. The CPU hardware looks at the top 10 bits of `0x08048000` (which is `0x020`).
2. It looks at index `32` in the active Page Directory (pointed to by `CR3`).
3. It extracts the physical address of the Page Table from that PDE.
4. It looks at the middle 10 bits of `0x08048000` (which is `0x048`).
5. It looks at index `72` in that Page Table.
6. It checks the flags. If `PTE_P` is 0, it triggers a Page Fault. If `PTE_U` is 0 (kernel only) but the CPU is in user mode, it triggers a Page Fault.
7. If all is well, it takes the physical page address from the PTE, adds the bottom 12 bits (`0x000`), and writes to RAM.

All of this happens in hardware, instantly!

## 6. Edge Cases, Failure Modes & Resilience

| Scenario | Consequence | Prevention |
|---|---|---|
| Passing a user pointer to `vtop()` | Assertion failure | `vtop()` asserts `is_kernel_vaddr(vaddr)`. It only works for addresses >= 0xC0000000. |
| Accessing unmapped memory | Page Fault (#PF) | The hardware traps to the OS. In Pintos user mode, this kills the process. |

## 7. Hands-On Verification & Challenges

<details>
<summary><strong>Q1: Why does <code>vtop()</code> only work for kernel addresses? How do you translate a user virtual address to a physical address?</strong></summary>

The kernel virtual space is identity mapped (virtual = physical + 0xC0000000), making the math trivial. User virtual addresses are mapped arbitrarily via page tables. To translate a user address, you must manually walk the page tables using `pd_no()`, `pt_no()`, and `pagedir_get_page()`.
</details>

## 8. Lab Modifications Guide

These files are **not modified in any lab**. 
However, you will use `pg_round_down()` frequently (especially when debugging threads), and in **Project 2/3**, you will use `is_user_vaddr()` to validate pointers passed by user programs via system calls.
