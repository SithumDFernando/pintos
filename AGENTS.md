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
3. **Incremental Implementation & Phase Completion Protocol**:
   - Implement one phase at a time following the established phase dependency order:
     1. Design Document Initialization (`pintos/src/userprog/DESIGNDOC`)
     2. Argument Passing (`pintos/src/userprog/process.c`)
     3. Syscall Dispatcher, FD Table & FS Lock (`pintos/src/userprog/syscall.c`, `pintos/src/threads/thread.h`)
     4. User Memory Access & Validation (`pintos/src/userprog/syscall.c`, `pintos/src/userprog/exception.c`)
     5. Process Control & Synchronization (`exec`, `wait`, `exit` via shared `child_status` in `pintos/src/userprog/process.c`, `pintos/src/threads/thread.[ch]`)
     6. File System Syscalls (`create`, `open`, `read`, `write`, `close`, etc. in `pintos/src/userprog/syscall.c`)
     7. Denying Writes to Running Executables (`pintos/src/userprog/process.c`)
     8. Full Test Suite & Final Submission Package
   - **MANDATORY PAUSE AFTER EACH PHASE**:
     - Stop tool execution immediately after completing and verifying the phase.
     - Update [docs/dev/worksummary.md](file:///c:/Users/sithu/MyWorks/UOM/Sem_3/OS/pintos_240159J/docs/dev/worksummary.md) recording:
       - Phase name & objective.
       - Exact files modified (with paths relative to workspace root).
       - Key design decisions & trade-offs made.
       - Test verification results.
     - Provide the user with:
       - A descriptive **git commit message**.
       - A **ready-to-paste prompt** for initiating the next phase.

4. **Testing & Verification**:
   - Run individual test targets (e.g., `make tests/userprog/args-single.result`) before executing the full suite (`make check`).
   - Clean up all temporary debugging `printf` statements and `hex_dump` calls before test verification.
   - Capture test summaries for required Moodle submission deliverables.

---

## 4. Kernel Architecture & Coding Standards
- **Path Reference Rule**:
  - In all documentation, plans, reports, messages, and code comments, always use relative paths originating from the workspace root (e.g., `pintos/src/userprog/process.c`, `pintos/src/threads/thread.h`, `docs/dev/02-lab2-phase1-argument-passing.md`).
  - Never use naked/isolated file names (e.g. avoid `process.c`) without their containing path from the root, except when discussing commands executed strictly inside a specific container directory.
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
