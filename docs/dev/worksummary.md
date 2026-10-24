# Pintos Lab 2 Implementation Work Summary

This document records the incremental progress, architectural decisions, file changes, and test outcomes for each implementation phase of **Lab 2: User Programs & System Calls**.

---

## Log of Completed Phases

| Phase | Title | Status | Files Modified | Key Test Targets |
|---|---|---|---|---|
| Phase 0 | Design Document Initialization | ✅ Done | `pintos/src/userprog/DESIGNDOC` | — |
| Phase 1 | Argument Passing | ✅ Done (manual stack-dump verified) | `pintos/src/userprog/process.c` | `args-none` ... `args-dbl-space` (5) — need Phases 2/4 to pass |
| Phase 2 | Syscall Dispatcher, FD Table & FS Lock | ⏳ Pending | `userprog/syscall.c`, `threads/thread.h` | `halt`, `exit` |
| Phase 3 | User Memory Access & Safety | ⏳ Pending | `userprog/syscall.c`, `userprog/exception.c` | `sc-*`, `bad-*`, `*-boundary` |
| Phase 4 | Process Control (exec / wait / exit) | ⏳ Pending | `userprog/process.c`, `threads/thread.[ch]` | `exec-*`, `wait-*`, `multi-*` |
| Phase 5 | File System Syscalls | ⏳ Pending | `userprog/syscall.c` | `create-*`, `open-*`, `read-*`, `write-*`, etc. |
| Phase 6 | Deny Writes to Running Executables | ⏳ Pending | `userprog/process.c` | `rox-*` |
| Phase 7 | Full Test Suite Verification & Submission | ⏳ Pending | Full workspace | All 76 tests in `tests/userprog` |

---

## Phase Details & Architectural Decisions

### Phase 0 + Phase 1 — Design Doc Init & Argument Passing

**Files modified**
- `pintos/src/userprog/DESIGNDOC` (new; standard P2 template, Argument Passing section A1–A4 filled, System Calls section marked pending).
- `pintos/src/userprog/process.c`.

**Implementation**
- `extract_prog_name()` helper: copies first token into a caller buffer (no static state).
- `process_execute()`: thread named after program only; empty cmdline → `TID_ERROR`; full cmdline copy still goes to `start_process`.
- `load()`: takes the full cmdline, opens program name only (so `load: <prog>: open failed` is correct), passes cmdline to `setup_stack()`.
- `setup_stack()`/`push_arguments()`: tokenize a private copy with `strtok_r`, size check before writing, push strings, align, NULL, argv pointers, argv, argc, fake return address.

**Design decisions / trade-offs**
- Stack is filled via the kernel mapping (`kpage`) *before* `install_page()`: independent of the active page directory and failure only frees `kpage`.
- Three scratch pages (`palloc`) instead of local arrays to protect the 4 KB kernel stack; all freed on every path.
- Conservative overflow check (assumes 3 padding bytes) rejects oversized commands cleanly rather than crashing.

**Verification (Docker)**
- `make` in `/pintos/src/userprog`: builds with no warnings or errors.
- Temporary `hex_dump` + sleeping `process_wait` run of `args-many a b  c d`: argc=5, argv=0xbfffffd4, NULL sentinel, double space collapsed, 4-byte aligned `*esp`, fake return address 0. Temporary debug code then **removed**.
- Official `args-*` tests are not run yet: they need `write`/`exit` (Phase 2) and a real `process_wait` (Phase 4); `process_wait()` still returns -1 and the syscall handler is still the stub.

