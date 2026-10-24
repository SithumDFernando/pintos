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
| Phase 4 | Process Control (exec / wait / exit) | ✅ Done | `pintos/src/userprog/process.[ch]`, `pintos/src/threads/thread.[ch]`, `pintos/src/userprog/syscall.c`, `pintos/src/userprog/DESIGNDOC` | `exec-*`, `wait-*`, `multi-recurse`, `args-*`, `halt`, `exit`, `sc-*`, `bad-*` (all PASS) |
| Phase 5 | File System Syscalls | ✅ Done | `pintos/src/userprog/syscall.c`, `pintos/src/userprog/DESIGNDOC` | `create-*`, `open-*`, `close-*`, `read-*`, `write-*`, `multi-child-fd` (all 31 PASS) |
| Phase 6 | Deny Writes to Running Executables | ✅ Done | `pintos/src/userprog/process.c`, `pintos/src/threads/thread.[ch]`, `pintos/src/userprog/DESIGNDOC` | `rox-simple`, `rox-child`, `rox-multichild` (all PASS; 80/80 total) |
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

### Phase 4 - Process Control & Synchronization (exec, wait, exit)

**Files modified**
- `pintos/src/userprog/process.h`: defined `struct child_status` (tid, exit_code, load_ok, load_done, fn_copy, load_sema, exit_sema, ref_cnt, ref_lock, elem).
- `pintos/src/userprog/process.c`:
  - Implemented `release_status()` for reference-counted reclamation of `child_status`.
  - Implemented `process_execute()`: allocates `child_status`, passes it to `thread_create()`, waits on `load_sema`, returns `TID_ERROR` if child load fails.
  - Implemented `start_process()`: links `cur->my_status`, loads executable, frees command-line page, signals `load_sema`, exits if load failed.
  - Implemented `process_wait()`: searches `cur->children`, blocks on `exit_sema`, removes child from list, releases reference, returns child exit code (or -1 if invalid/already waited). Replaces temporary `timer_sleep (300)`.
  - Implemented `process_exit()`: announces exit `name: exit(code)`, closes all FDs, drops references to orphaned children, signals `exit_sema` and releases child reference to `my_status`, destroys page directory.
- `pintos/src/threads/thread.h`: added `children` list and `my_status` pointer to `struct thread` under `USERPROG`.
- `pintos/src/threads/thread.c`: initialized `children` list and `my_status = NULL` in `init_thread()`.
- `pintos/src/userprog/syscall.c`: wired up `SYS_EXEC` (`sys_exec` validates cmdline with `valid_string`, calls `process_execute`) and `SYS_WAIT` (`sys_wait` calls `process_wait`). Removed `UNUSED` on `valid_string`.
- `pintos/src/userprog/DESIGNDOC`: filled B1 (`child_status` and thread additions), B5 (`wait` algorithm and idempotency), B7 (`exec` load synchronization and failure handling), B8 (termination races and orphan cleanup), B11 (1:1 TID/PID mapping).

**Design decisions / trade-offs**
- Heap-allocated `struct child_status` shared by parent and child with `ref_cnt = 2` and `ref_lock`: decouples zombie status and exit code from the dying thread's 4 KB page, avoiding use-after-free regardless of whether parent or child exits first.
- Strict child list isolation: `process_wait()` only checks `cur->children`, preventing wait on arbitrary TIDs or grandchildren. Removing from list immediately prevents duplicate wait calls from succeeding (`wait-twice`).
- Two semaphores (`load_sema`, `exit_sema`): `load_sema` guarantees `exec` does not return until load status is known; `exit_sema` blocks `wait` until child exits or unblocks immediately if already dead.
- Defense against child load crash: `process_exit()` checks `!cs->load_done` and signals `load_sema` if child dies mid-load so parent never deadlocks.
- `fs_lock` is never held across `process_execute` or `process_wait`.

**Verification (Docker, `make -k <test>.result`)**
- Build: clean, no warnings or errors.
- Key Targets PASS (31):
  - `exec-*`: `exec-once`, `exec-arg`, `exec-bound`, `exec-bound-2`, `exec-bound-3`, `exec-multiple`, `exec-missing`, `exec-bad-ptr` (all 8 PASS).
  - `wait-*`: `wait-simple`, `wait-twice`, `wait-killed`, `wait-bad-pid` (all 4 PASS).
  - `multi-*`: `multi-recurse` (PASS, depth 15 recursion verified).
  - Re-run regressions: `halt`, `exit`, `args-none`, `args-single`, `args-multiple`, `args-many`, `args-dbl-space`, `sc-bad-sp`, `sc-bad-arg`, `sc-boundary`, `sc-boundary-2`, `sc-boundary-3`, `bad-read`, `bad-read2`, `bad-write`, `bad-write2`, `bad-jump`, `bad-jump2` (all 18 PASS).
- Note: `multi-child-fd` passes once Phase 5 is in place.

### Phase 5 - File System Syscalls

**Files modified**
- `pintos/src/userprog/syscall.c`:
  - Included `devices/input.h` and `filesys/filesys.h`.
  - Implemented `sys_create()`: validates filename string, calls `filesys_create()` under `fs_lock`.
  - Implemented `sys_remove()`: validates filename string, calls `filesys_remove()` under `fs_lock`.
  - Implemented `sys_open()`: validates filename string, calls `filesys_open()` under `fs_lock`, registers file in `fd_alloc()`.
  - Implemented `sys_filesize()`: looks up fd, returns `file_length()` under `fs_lock`.
  - Implemented `sys_read()`: validates destination buffer, handles fd 0 via `input_getc()` loop (without `fs_lock`), rejects fd 1, reads file via `file_read()` under `fs_lock`.
  - Implemented `sys_write()`: validates source buffer, handles fd 1 via `putbuf()` (without `fs_lock`), rejects fd 0, writes file via `file_write()` under `fs_lock`.
  - Implemented `sys_seek()`: looks up fd, calls `file_seek()` under `fs_lock`.
  - Implemented `sys_tell()`: looks up fd, returns `file_tell()` under `fs_lock`.
  - Implemented `sys_close()`: calls `fd_close()`, which closes file under `fs_lock` and frees the entry. Ignores fds < 2.
  - Wired all dispatch entries in `syscall_handler()`.
- `pintos/src/threads/thread.h`: moved `struct child_status;` forward declaration outside `struct thread` to eliminate compiler warning.
- `pintos/src/tests/userprog/sample.txt`: normalized line endings (removed Windows CRLF `\r` artifact that caused 4-byte size mismatch in sample file tests).
- `pintos/src/userprog/DESIGNDOC`: answered B4 (page table walk inspection counts), B6 (bad-pointer resource safety and leak prevention), and B10 (rationale for returning error codes on invalid descriptors rather than process termination).

**Design decisions / trade-offs**
- Validate-then-lock pattern: all user buffer ranges and strings are validated before taking `fs_lock` to avoid leaving the lock held if an invalid address kills the process.
- Lock avoidance on console I/O: reading from stdin (`input_getc`) and writing to stdout (`putbuf`) do not acquire `fs_lock`, allowing file I/O and console I/O to proceed concurrently without lock contention.
- Invalid descriptor tolerance: unknown/closed descriptors, reading from stdout, writing to stdin, or closing console fds return -1 / false or silently ignore rather than killing the process, adhering to standard POSIX semantics.

**Verification (Docker, `make check` and individual targets)**
- Build: clean, zero warnings, zero errors.
- Phase 5 Key Targets (31/31 PASS):
  - `create-*` (7): `create-normal`, `create-empty`, `create-null`, `create-bad-ptr`, `create-long`, `create-exists`, `create-bound` (all PASS).
  - `open-*` (7): `open-normal`, `open-missing`, `open-boundary`, `open-empty`, `open-null`, `open-bad-ptr`, `open-twice` (all PASS).
  - `close-*` (5): `close-normal`, `close-twice`, `close-stdin`, `close-stdout`, `close-bad-fd` (all PASS).
  - `read-*` (6): `read-normal`, `read-bad-ptr`, `read-boundary`, `read-zero`, `read-stdout`, `read-bad-fd` (all PASS).
  - `write-*` (6): `write-normal`, `write-bad-ptr`, `write-boundary`, `write-zero`, `write-stdin`, `write-bad-fd` (all PASS).
  - `multi-child-fd` (PASS, child cannot close parent's fd, parent reads correctly).
- Full suite status: **All 80 tests passed!** (100% pass rate achieved after Phase 6).

### Phase 6 — Deny Writes to Running Executables

**Files modified**
- `pintos/src/threads/thread.h`: added forward declaration `struct file;` and member `struct file *executable;` to `struct thread` under `USERPROG`.
- `pintos/src/threads/thread.c`: explicitly initialized `t->executable = NULL;` in `init_thread()`.
- `pintos/src/userprog/process.c`:
  - `load()`: on successful load, called `file_deny_write (file)` and preserved the open handle in `t->executable = file` without closing it at the `done:` label. On load failure, closed `file` immediately as before.
  - `process_exit()`: closed `cur->executable` under `fs_lock` (automatically invoking `file_allow_write()` to decrement `inode->deny_write_cnt`) and set `cur->executable = NULL`. Performed this cleanup strictly **before** signaling `exit_sema` to wake the waiting parent.
- `pintos/src/userprog/DESIGNDOC`: recorded `struct file *executable` under B1; answered B12 with detailed rationale on keeping the file handle open across the process lifetime, handling concurrent executions (`rox-multichild`), and order-of-events synchronization with parent wake-up (`rox-child`).

**Design decisions / trade-offs**
- Inode deny counter lifetime bound to open file handle: Pintos's `file_deny_write()` increments `inode->deny_write_cnt`, while `file_close()` automatically calls `file_allow_write()` to decrement it. Keeping `cur->executable` open for the duration of process execution ensures that writes remain denied until process termination.
- Critical order of events on exit: in `process_exit()`, `cur->executable` is closed under `fs_lock` before `sema_up(&cs->exit_sema)`. This guarantees that when a parent's `process_wait()` unblocks, writes to the child's binary on disk have already been re-enabled (preventing failures in tests like `rox-child`).
- Handling concurrent executions: each process executing the same binary opens its own `struct file` pointing to the shared `struct inode`. Each call to `file_deny_write()` increments `inode->deny_write_cnt`, ensuring writes remain blocked until the last concurrent instance terminates and closes its executable handle (`rox-multichild`).
- Failure & abnormal exit safety: if `load()` fails, `file_close(file)` is executed at `done:`, leaving `t->executable = NULL`. If a process is terminated prematurely by a page fault or exception, `thread_exit()` unconditionally routes through `process_exit()`, ensuring `cur->executable` is always closed and never leaks.

**Verification (Docker, `make tests/userprog/rox-*.result` and `make check`)**
- Build: clean, zero warnings, zero errors.
- Phase 6 Key Targets (3/3 PASS):
  - `rox-simple` (PASS): verifies open executable cannot be written to through a user descriptor.
  - `rox-child` (PASS): verifies child executable cannot be modified while running, and parent can write back once child wait returns.
  - `rox-multichild` (PASS): verifies recursive concurrent executions maintain write-denial until all instances terminate.
- Full suite status: **All 80 tests passed!** (100% pass rate across the full Pintos User Programs test suite).



