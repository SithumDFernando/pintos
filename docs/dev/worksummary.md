# Pintos Lab 2 Implementation Work Summary

This document records the incremental progress, architectural decisions, file changes, and test outcomes for each implementation phase of **Lab 2: User Programs & System Calls**.

---

## Log of Completed Phases

| Phase | Title | Status | Files Modified | Key Test Targets |
|---|---|---|---|---|
| Phase 0 | Design Document Initialization | ✅ Done | `pintos/src/userprog/DESIGNDOC` | — |
| Phase 1 | Argument Passing | ✅ Done (manual stack-dump verified) | `pintos/src/userprog/process.c` | `args-none` ... `args-dbl-space` (5) — need Phases 2/4 to pass |
| Phase 2 | Syscall Dispatcher, FD Table & FS Lock | ✅ Done | `pintos/src/userprog/syscall.[ch]`, `pintos/src/threads/thread.[ch]`, `pintos/src/userprog/process.c` | `halt`, `exit`, `args-*` (all PASS) |
| Phase 3 | User Memory Access & Safety | ✅ Done | `pintos/src/userprog/syscall.c`, `pintos/src/userprog/exception.c`, `pintos/src/userprog/DESIGNDOC` | `sc-*`, `bad-*`, `args-*`, `halt`, `exit` PASS; 7 boundary/file tests await later phases |
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

### Phase 2 - Syscall Dispatcher, FD Table & FS Lock

**Files modified**
- `pintos/src/userprog/syscall.c` (rewritten), `pintos/src/userprog/syscall.h`
- `pintos/src/threads/thread.h`, `pintos/src/threads/thread.c` (`exit_code`, `fd_list`, `next_fd`; initialised in `init_thread`)
- `pintos/src/userprog/process.c` (exit message + `fd_close_all()` in `process_exit`; `fs_lock` held during `load`; temporary `process_wait`)
- `pintos/src/userprog/DESIGNDOC` (System Calls: B1, B2, B3 partial, B9)

**Implementation**
- Dispatcher: validates the syscall-number word and its N argument words, copies args to a kernel array, rejects numbers outside `SYS_HALT..SYS_CLOSE` with `exit(-1)`.
- Implemented now: `halt`, `exit`, `write` (fd 1 via `putbuf`; other fds return -1 until the file-syscall phase). All other calls currently `exit(-1)`.
- `fs_lock` (global), `fd_alloc/fd_lookup/fd_close/fd_close_all` (list of `struct fd_entry`, not inherited).
- `process_exit` prints `name: exit(N)` for processes with a page directory; default `exit_code` is -1 so killed/failed-load processes report -1.

**Design decisions / trade-offs**
- FD list instead of array: +12 B in `struct thread` vs +512 B, no fd limit; O(n) lookup is fine.
- Exit message pulled forward from the process-control phase because every `args-*` test checks it.
- `user_range_ok()` here is a minimal page-mapped check needed to read args safely; the next phase hardens it (strings, buffers, fault handling).
- **TEMPORARY**: `process_wait()` does `timer_sleep (300)` and returns -1 so the kernel does not power off early. Must be replaced in Phase 4.

**Verification (Docker)**
- `make` builds clean (only the pre-existing `lib/kernel/debug.c` frame-address warning).
- `make -k tests/userprog/{halt,exit,args-none,args-single,args-multiple,args-many,args-dbl-space}.result`: all 7 PASS.

### Phase 3 - User Memory Access & Validation

**Files modified**
- `pintos/src/userprog/syscall.c`: `user_range_ok()` replaced by `valid_byte()`, `valid_range()`, `valid_string()`; `sys_write` and the dispatcher use `valid_range()`.
- `pintos/src/userprog/exception.c`: `kill()` sets `exit_code = -1` before `thread_exit()`; a kernel-mode fault on a user address kills the process instead of panicking; added `threads/vaddr.h` include.
- `pintos/src/userprog/DESIGNDOC` (B3 completed).

**Design decisions / trade-offs**
- Verify-then-dereference (not `get_user`/`put_user`): simpler to reason about and document; costs a page-table lookup per page.
- `valid_range()` checks both ends, wrap-around, and one byte per intermediate page; zero length accepted.
- `valid_string()` is bounded to one page, re-checking only at page starts. It is marked `UNUSED` until `exec`/`create`/`open`/`remove` land in later phases.
- Validation always precedes `fs_lock`, so killed processes never hold it.

**Verification (Docker, `make -k tests/userprog/<t>.result`)**
- Build: no new warnings.
- PASS (28): `halt`, `exit`, `args-{none,single,multiple,many,dbl-space}`, `sc-bad-sp`, `sc-bad-arg`, `sc-boundary{,-2,-3}`, `bad-read{,2}`, `bad-write{,2}`, `bad-jump{,2}`, `write-bad-ptr`, `read-bad-ptr`, `create-bad-ptr`, `open-bad-ptr`, `exec-bad-ptr`, `create-null`, `open-null`, `write-stdin`, `read-stdout`.
- FAIL, expected (7), need unimplemented syscalls: `create-bound`, `open-boundary`, `read-boundary`, `write-boundary`, `write-normal`, `write-zero` (need `create`/`open`/`read`/`close`) and `exec-bound` (needs `exec`/`wait`). These cannot be judged until Phases 4-5.
- Note: `*-bad-ptr`/`*-null` pass now because the unimplemented call exits -1; they must be re-run once the real syscalls exist.

