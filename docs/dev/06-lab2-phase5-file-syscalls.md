# Lab 2 / Phase 5 — File System Syscalls

**Files**: `pintos/src/userprog/syscall.c`
**Tests**: `create-*` (7), `open-*` (7), `close-*` (5), `read-*` (6), `write-*` (6)
**Depends on**: Phase 2 (validation), Phase 3 (fd table, `fs_lock`)

Uses the existing API (do not modify `pintos/src/filesys/`): `filesys_create(name, size)`,
`filesys_remove(name)`, `filesys_open(name)`, `file_read/write/seek/tell/length/close`.

## 1. Per-Syscall Specification

Pattern: `validate → lock → call → unlock → result`.

| Syscall | Validation | Behaviour | Failure result |
|---|---|---|---|
| `create(file,size)` | `valid_string(file)` | `filesys_create` under lock | `false` (e.g. duplicate, name > 14 chars); bad ptr ⇒ exit(-1) |
| `remove(file)` | `valid_string` | `filesys_remove` under lock | `false` |
| `open(file)` | `valid_string` | `filesys_open`; `NULL` ⇒ -1; else `fd_alloc` | `-1` (missing, empty name `""`) |
| `filesize(fd)` | — | `file_length` under lock | `-1` for bad fd |
| `read(fd,buf,n)` | `valid_range(buf,n)` | fd0: loop `input_getc` n times; fd1 ⇒ -1; else `file_read` | `-1` (bad fd) |
| `write(fd,buf,n)` | `valid_range(buf,n)` | fd1: `putbuf(buf,n)`; fd0 ⇒ -1; else `file_write` | `-1` (bad fd) |
| `seek(fd,pos)` | — | `file_seek` (ignore bad fd) | — |
| `tell(fd)` | — | `file_tell` | `-1`/`0xFFFFFFFF` bad fd |
| `close(fd)` | — | lookup; `file_close`; remove+free entry | bad/closed/0/1 ⇒ ignore **or** exit(-1) (tests accept both) |

Test-driven decisions (verified in `.ck` files):
- `create-empty`: `create("")` returns false ⇒ acceptable (`create(""): 0`); alternative exit(-1) also accepted.
- `open-empty`: must return `-1`.
- `write-stdin`, `read-stdout`, `close-stdin/stdout`, `*-bad-fd`: *either* silent failure *or* exit(-1). Choice: **return -1 / ignore** (no process death) — lets a program continue after a mistake; document in design doc.
- `write-zero`/`read-zero`: return 0.
- `read-normal` etc. need `pintos/src/tests/userprog/sample.txt` installed (handled by `pintos/src/tests/userprog/Make.tests` `PUTFILES`).

## 2. Return-Value Details
- `read`: bytes actually read (may be < n at EOF); `0` at EOF.
- `write` to a file: Pintos files **cannot grow** — may return fewer bytes than requested (`write-normal` writes within file size). Return `file_write`'s value.
- `write` to fd1: for large buffers, call `putbuf` once (simplest) or in ≤ 256 B chunks to reduce interleaving between processes; chosen: single `putbuf` (output of one `write` is atomic w.r.t. other threads because console uses a lock).
- Negative/huge `size`: `size` is `unsigned`; `valid_range` handles wrap; `n > INT_MAX` ⇒ treat as invalid (exit(-1)).

## 3. `fd` Edge Cases (`read-bad-fd`, `write-bad-fd`, `close-bad-fd`)
`fd` is `int` from the user; values `-5, INT_MIN, INT_MAX, 0x20101234, 7`:
`fd_lookup` returns NULL for anything not in the list (no array indexing ⇒ no overflow) — the main advantage of the list design.

## 4. Double Close / Reuse
- `close` removes the entry ⇒ second close finds nothing (`close-twice` ✅).
- `open-twice`: two `open` of same file return **different** fds with independent `file` objects (`filesys_open` creates a new `struct file` each time; shared inode).
- `fd` numbers are never reused within a process (monotonic `next_fd`) — avoids stale-fd confusion.

## 5. Implementation Steps
1. Implement `fd_alloc / fd_lookup / fd_remove` helpers (Phase 3).
2. Implement handlers one by one in test order: `create` → `open` → `close` → `read` → `write` → `seek/tell/filesize` → `remove`.
3. Build user programs (`make` in `pintos/src/examples/` is not needed for tests; tests are built by `make check`).
4. Run groups: `make tests/userprog/create-normal.result` etc. (in container).

## 6. Pitfalls
- ⚠️ Do **not** keep `fs_lock` held when calling `thread_exit()` on a bad pointer (validate first).
- ⚠️ `filesys_open` allocates; check NULL.
- ⚠️ `input_getc` blocks — fine in syscall context (interrupts on).
- ⚠️ `file_write` may return 0 if `deny_write` set (Phase 6) — return it directly.
- ⚠️ Name length: `create-long` expects `false` (name longer than `NAME_MAX`), not a crash.

## 7. Acceptance
✅ 31 tests (`create*`, `open*`, `close*`, `read*`, `write*`) pass.

## 8. Design-Doc Material
Rationale for returning -1 vs terminating on bad fd; global lock granularity;
why list-based fd table.
