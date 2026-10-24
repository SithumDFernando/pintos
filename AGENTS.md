# Agent Guidelines — Pintos OS Lab Development

## 1. Project Context & Objectives
- **Module**: CS2043 Operating Systems (Semester 3, University of Moratuwa).
- **Target**: Completion, documentation, and verification of Pintos Labs (primarily **Lab 2: User Programs & System Calls**, with reference plans for Lab 0 and Lab 1).
- **Status of Learning Materials**: All conceptual and architectural materials in `docs/learn/` are **complete**. Do not generate or recreate learning docs. Focus exclusively on lab design documents, kernel code implementation, and Docker-based testing.

---

## 2. Environment & Execution Rules (Docker Only)
- **Containerized Environment**: Pintos compilation, disk image creation, and simulation run strictly inside Docker (via the project `Dockerfile` and `pintos` image).
- **No Native VM / Toolchain Assumptions**: Never assume native x86 GCC, QEMU, or Bochs are installed directly on the host Windows system.
- **Docker Mount**: The source code is mounted from the host into `/pintos/src` in the container:
  ```bash
  docker run -it --rm -v "${PWD}/pintos/src:/pintos/src" pintos
  ```
- **Working Directories in Container**:
  - Lab 2 (User Programs): `/pintos/src/userprog`
  - Lab 1 (Threads): `/pintos/src/threads`
- **Building and Running Tests**:
  - Always execute `make` and `make check` inside the container environment.
  - Test results are verified against test assertions (`tests/userprog/*.ck`).

---

## 3. Strict Development Workflow
Always proceed in the following ordered steps:
1. **Implementation Plans** (`docs/dev/`):
   - Reference and maintain granular phase plans ([docs/dev/README.md](file:///c:/Users/sithu/MyWorks/UOM/Sem_3/OS/pintos_240159J/docs/dev/README.md)).
2. **Design Document** (`DESIGNDOC`):
   - Formulate and update the formal design document (e.g., `pintos/src/userprog/DESIGNDOC`) before finalizing implementations.
   - Provide clear justifications for data structures, algorithms, synchronisation primitives, and trade-offs.
3. **Incremental Implementation**:
   - Follow the established phase dependency order:
     1. Argument Passing (`process.c`)
     2. Syscall Dispatcher, FD Table & FS Lock (`syscall.c`, `thread.h`)
     3. User Memory Access & Validation (`syscall.c`, `exception.c`)
     4. Process Control & Synchronization (`exec`, `wait`, `exit` via shared `child_status`)
     5. File System Syscalls (`create`, `open`, `read`, `write`, `close`, etc.)
     6. Denying Writes to Running Executables (`load()`, `file_deny_write()`)
4. **Testing & Verification**:
   - Run individual test targets (e.g., `make tests/userprog/args-single.result`) before executing the full suite (`make check`).
   - Clean up all temporary debugging `printf` statements and `hex_dump` calls before test verification.
   - Capture test summaries for required Moodle submission deliverables.

---

## 4. Kernel Architecture & Coding Standards
- **Kernel Stack Limits**:
  - Each thread page is 4 KB total (struct thread at bottom, stack growing down from top).
  - Never allocate large arrays or buffers as local stack variables.
  - Keep `struct thread` compact (well under 1 KB).
- **Memory & Resource Safety**:
  - Validate every user pointer, buffer range, and string before dereferencing in kernel mode.
  - Invalid user memory access must terminate the process with exit status `-1`, never panic the kernel.
  - Every allocated resource (`malloc`, `palloc`, `file *`, `fd_entry`, `child_status`) must be freed on all termination paths (normal exit or killed).
- **Synchronization & Deadlock Prevention**:
  - The Pintos file system is not thread-safe: all file system calls must be protected by a global `fs_lock`.
  - Validate user memory *before* acquiring `fs_lock`.
  - Never hold `fs_lock` while sleeping or waiting on process creation/child termination (`process_execute`, `process_wait`).
- **Academic Integrity**:
  - Code and design justifications must be original, clean, well-commented, and adhere to university guidelines against plagiarism.
