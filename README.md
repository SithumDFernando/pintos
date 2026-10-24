# Pintos Operating System — CS2043 Operating Systems

**Department of Computer Science & Engineering, University of Moratuwa**  
**Semester 3 | Individual Project**

- **Student Name**: Fernando H. T. S. D.
- **Index / University ID**: 240159J
- **Email**: [sithum.24@cse.mrt.ac.lk](mailto:sithum.24@cse.mrt.ac.lk)
- **Project Target**: Lab 2 — User Programs & System Calls
- **Final Verification**: **80 / 80 tests passed (100% pass rate)**

---

## 1. Project Overview

This repository contains the complete implementation, design documentation, and Docker verification suite for **Pintos Lab 2: User Programs & System Calls** for the CS2043 Operating Systems module.

### Implemented Subsystems & Phases

1. **Phase 1: Argument Passing** (`pintos/src/userprog/process.c`)
   - Safe parsing of command-line arguments using reentrant `strtok_r()`.
   - Stack frame construction on the mapped user stack under `PHYS_BASE`.
   - Word alignment (4-byte), NULL pointer sentinel, `argv` pointer vector, `argv`, `argc`, and fake return address 0.
   - Comprehensive stack overflow check rejecting oversized commands cleanly.

2. **Phase 2 & 3: Syscall Dispatcher & Memory Safety** (`pintos/src/userprog/syscall.c`, `pintos/src/userprog/exception.c`)
   - Syscall stack validation and dispatch mechanism for 13 system calls (`halt` through `close`).
   - Rigorous user memory access validation using verify-then-dereference (`valid_byte`, `valid_range`, `valid_string`).
   - Page fault handler backstop terminating invalid accesses with exit code `-1` instead of kernel panic.

3. **Phase 4: Process Control & Synchronization** (`pintos/src/userprog/process.c`, `pintos/src/threads/thread.[ch]`)
   - Synchronization of process creation (`process_execute`) and ELF binary loading via `load_sema`.
   - Process waiting (`process_wait`) and termination (`process_exit`) synchronized via heap-allocated, reference-counted `struct child_status`.
   - Full protection against parent/child termination races and orphan resource leaks.

4. **Phase 5: File System System Calls** (`pintos/src/userprog/syscall.c`)
   - Implementation of file descriptors (`fd_alloc`, `fd_lookup`, `fd_close`, `fd_close_all`) dynamically allocated per-process.
   - Full support for `create`, `remove`, `open`, `filesize`, `read`, `write`, `seek`, `tell`, and `close`.
   - POSIX-compliant handling of stdin (fd 0) and stdout (fd 1).
   - Global `fs_lock` serializing non-reentrant Pintos file system operations.

5. **Phase 6: Deny Writes to Running Executables** (`pintos/src/userprog/process.c`, `pintos/src/threads/thread.h`)
   - Preserving open executable handle in `t->executable` upon successful ELF load and invoking `file_deny_write()`.
   - Re-enabling writes upon process exit via `file_close()` before waking up waiting parent threads (`rox-*`).

6. **Phase 7: Full Verification & Design Document** (`pintos/src/userprog/DESIGNDOC`)
   - Formal Stanford / CS2043 design document containing detailed design justifications, data structure definitions, stack layout diagram, and trade-off analyses.

---

## 2. Directory Structure

```text
pintos_240159J/
├── Dockerfile                  # Containerized toolchain environment (Ubuntu 18.04 + i386-elf-gcc + QEMU)
├── README.md                   # Project overview & instructions (this file)
├── AGENTS.md                   # Kernel development guidelines and rules
├── docs/
│   ├── dev/                    # Incremental phase development plans & work summaries
│   │   ├── README.md           # Implementation phase index & dependencies
│   │   ├── worksummary.md      # Detailed phase-by-phase implementation log
│   │   └── 0*-lab2-*.md        # Granular phase specifications (Phases 1 - 7)
│   └── labs/                   # Course lab briefs and Moodle instructions
└── pintos/
    └── src/
        ├── userprog/           # User program loading, syscalls, & DESIGNDOC
        │   ├── DESIGNDOC       # Formal Design Document for Lab 2
        │   ├── process.c       # Argument passing, ELF loader, exec/wait lifecycle
        │   ├── syscall.c       # System call dispatcher, memory validation, file syscalls
        │   └── exception.c     # Page fault handler & fault termination backstop
        ├── threads/            # Kernel threading, scheduling, and sync primitives
        │   ├── thread.h        # PCB structure (struct thread) with USERPROG members
        │   └── thread.c        # Thread initialization and state transitions
        ├── filesys/            # Pintos basic file system implementation
        └── tests/userprog/     # Lab 2 automated test suite (80 test cases)
```

---

## 3. Environment & Execution (Docker Only)

Pintos requires an x86 cross-compiler and QEMU simulator provided within the Docker container. All compilation and testing must be performed inside Docker.

### 3.1 Build the Docker Image

From the workspace root directory:

```bash
docker build -t pintos .
```

### 3.2 Run the Development Container

Start an interactive shell with the local source tree bind-mounted:

- **Linux / macOS / Git Bash**:
  ```bash
  docker run -it --rm -v "${PWD}/pintos/src:/pintos/src" pintos
  ```

- **Windows PowerShell**:
  ```powershell
  docker run -it --rm -v "${PWD}/pintos/src:/pintos/src" pintos
  ```

---

## 4. Building and Testing

Inside the container environment:

### 4.1 Compile the Kernel

```bash
cd /pintos/src/userprog
make
```

### 4.2 Run Individual Tests

To run a specific test target (e.g. `args-multiple`, `exec-once`, `rox-child`):

```bash
cd /pintos/src/userprog/build
make tests/userprog/args-multiple.result
```

### 4.3 Run Full Test Suite

To run all 80 tests in the test suite:

```bash
cd /pintos/src/userprog/build
make check
```

---

## 5. Test Suite Verification Results

```text
pass tests/userprog/args-none
pass tests/userprog/args-single
pass tests/userprog/args-multiple
pass tests/userprog/args-many
pass tests/userprog/args-dbl-space
pass tests/userprog/sc-bad-sp
pass tests/userprog/sc-bad-arg
pass tests/userprog/sc-boundary
pass tests/userprog/sc-boundary-2
pass tests/userprog/sc-boundary-3
pass tests/userprog/halt
pass tests/userprog/exit
pass tests/userprog/create-normal
pass tests/userprog/create-empty
pass tests/userprog/create-null
pass tests/userprog/create-bad-ptr
pass tests/userprog/create-long
pass tests/userprog/create-exists
pass tests/userprog/create-bound
pass tests/userprog/open-normal
pass tests/userprog/open-missing
pass tests/userprog/open-boundary
pass tests/userprog/open-empty
pass tests/userprog/open-null
pass tests/userprog/open-bad-ptr
pass tests/userprog/open-twice
pass tests/userprog/close-normal
pass tests/userprog/close-twice
pass tests/userprog/close-stdin
pass tests/userprog/close-stdout
pass tests/userprog/close-bad-fd
pass tests/userprog/read-normal
pass tests/userprog/read-bad-ptr
pass tests/userprog/read-boundary
pass tests/userprog/read-zero
pass tests/userprog/read-stdout
pass tests/userprog/read-bad-fd
pass tests/userprog/write-normal
pass tests/userprog/write-bad-ptr
pass tests/userprog/write-boundary
pass tests/userprog/write-zero
pass tests/userprog/write-stdin
pass tests/userprog/write-bad-fd
pass tests/userprog/exec-once
pass tests/userprog/exec-arg
pass tests/userprog/exec-bound
pass tests/userprog/exec-bound-2
pass tests/userprog/exec-bound-3
pass tests/userprog/exec-multiple
pass tests/userprog/exec-missing
pass tests/userprog/exec-bad-ptr
pass tests/userprog/wait-simple
pass tests/userprog/wait-twice
pass tests/userprog/wait-killed
pass tests/userprog/wait-bad-pid
pass tests/userprog/multi-recurse
pass tests/userprog/multi-child-fd
pass tests/userprog/rox-simple
pass tests/userprog/rox-child
pass tests/userprog/rox-multichild
pass tests/userprog/bad-read
pass tests/userprog/bad-write
pass tests/userprog/bad-read2
pass tests/userprog/bad-write2
pass tests/userprog/bad-jump
pass tests/userprog/bad-jump2
pass tests/userprog/no-vm/multi-oom
pass tests/filesys/base/lg-create
pass tests/filesys/base/lg-full
pass tests/filesys/base/lg-random
pass tests/filesys/base/lg-seq-block
pass tests/filesys/base/lg-seq-random
pass tests/filesys/base/sm-create
pass tests/filesys/base/sm-full
pass tests/filesys/base/sm-random
pass tests/filesys/base/sm-seq-block
pass tests/filesys/base/sm-seq-random
pass tests/filesys/base/syn-read
pass tests/filesys/base/syn-remove
pass tests/filesys/base/syn-write
All 80 tests passed.
```

---

## 6. Submission Deliverables Summary

1. **Design Document**: Located at [`pintos/src/userprog/DESIGNDOC`](pintos/src/userprog/DESIGNDOC), detailing data structures, algorithms, synchronization logic, and trade-off justifications.
2. **Pintos Source Code**: All kernel modifications under [`pintos/src/threads/`](pintos/src/threads/) and [`pintos/src/userprog/`](pintos/src/userprog/).
3. **Test Summary**: Verifiable 100% pass rate (80/80 tests) generated by `make check`.
