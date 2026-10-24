# Lab 2 / Phase 6 — Denying Writes to Running Executables

**Files**: `pintos/src/userprog/process.c`, `pintos/src/threads/thread.h` (field `executable`)
**Tests**: `rox-simple`, `rox-child`, `rox-multichild` (uses `child-rox`)

## 1. Problem
A running program's file must not be modified. Today `load()` in `pintos/src/userprog/process.c` ends with
`file_close (file)` (line ~315), so nothing protects the binary.

## 2. Design
- `struct thread`: `struct file *executable;` (set in Phase 4's `init_thread` as `NULL`).
- In `load()`: on **success**, keep the file open (do **not** close at `done:`);
  call `file_deny_write (file)` and `t->executable = file`. On **failure**, close as today.
- In `process_exit()` (Phase 4 step 3): `lock_acquire(&fs_lock); file_close (cur->executable); lock_release(...)` — `file_close` calls `file_allow_write` internally if the file was denied, re-enabling writes.
- All fs calls inside `load()` are performed under `fs_lock` (Phase 3 rule).

Pseudo-change in `load()`:
```c
done:
  if (success) { file_deny_write (file); t->executable = file; }
  else         file_close (file);          // file may be NULL: file_close(NULL) is a no-op
```

## 3. Semantics (`pintos/src/filesys/inode.c`)
`deny_write_cnt` is **per inode**, so two processes running the same binary each
increment it; writes are re-allowed only after **both** close (`rox-multichild`).
Opening an executable for read via `open()` and writing through that fd
returns `0` bytes written (handled in `file_write → inode_write_at`).

## 4. Order-of-Events Guarantee (critical)
`rox-child`: parent writes to `child-rox` **after** `wait(child)` returns ⇒
the child must `file_close(executable)` **before** it ups `exit_sema`
(Phase 4: step 3 before step 5). Otherwise the final write fails with 0.

## 5. Edge Cases
| Case | Result |
|------|--------|
| load fails after `file_deny_write` was set | close in failure path ⇒ allow_write restored |
| process killed by fault | `thread_exit → process_exit` still closes executable |
| exec of same file twice concurrently | counter 2 → writes denied until both exit |
| `remove()` of running executable | allowed by Pintos (unlinked, data persists until closed) — no special handling needed |

## 6. Acceptance
✅ `rox-simple`, `rox-child`, `rox-multichild` pass; no regression on `exec-*`.

## 7. Pitfalls
- ⚠️ Keeping `file` open leaks if you forget it on the **exit** path ⇒ kernel runs out of memory over many `exec`s.
- ⚠️ `file_deny_write` before the header/segments are fully read is OK (reading is unaffected).
- ⚠️ Do not call `file_deny_write` while `file == NULL`.

## 8. Design-Doc Material
Why keep file open (inode deny counter lifetime = file handle lifetime);
ordering with parent wake-up.
