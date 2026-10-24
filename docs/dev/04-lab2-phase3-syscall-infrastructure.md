# Lab 2 / Phase 3 — Syscall Infrastructure, FD Table, FS Lock

**Files**: `pintos/src/userprog/syscall.[ch]`, `pintos/src/threads/thread.[ch]`, `pintos/src/userprog/process.[ch]`
**Tests unlocked**: `halt`, `exit`, plus a prerequisite for everything else.

## 1. Current State
`syscall_handler` = `printf ("system call!\n"); thread_exit ();` and ignores
`f->esp`. Syscall numbers: `pintos/src/lib/syscall-nr.h` (`SYS_HALT=0, SYS_EXIT=1, SYS_EXEC,
SYS_WAIT, SYS_CREATE, SYS_REMOVE, SYS_OPEN, SYS_FILESIZE, SYS_READ, SYS_WRITE,
SYS_SEEK, SYS_TELL, SYS_CLOSE`; higher numbers are project 3/4 — must be
rejected with exit(-1)).

## 2. Dispatcher Design

```c
static void syscall_handler (struct intr_frame *f) {
  uint32_t args[4];
  if (!read_word (f->esp, &args[0])) exit_bad ();   // syscall number
  int nargs = syscall_nargs (args[0]);              // table, -1 for unknown
  if (nargs < 0) exit_bad ();
  if (!valid_range (f->esp, 4 * (1 + nargs))) exit_bad ();
  memcpy (&args[1], (uint32_t *) f->esp + 1, 4 * nargs);
  switch (args[0]) { case SYS_HALT: ...; case SYS_WRITE: f->eax = sys_write(...); ... }
}
```

Arg-count table: `halt 0, exit 1, exec 1, wait 1, create 2, remove 1, open 1,
filesize 1, read 3, write 3, seek 2, tell 1, close 1`.
Return values go in `f->eax`. Handlers are small `static` functions with
typed parameters (`const char *`, `unsigned`, `int`).

## 3. Global File-System Lock

Pintos `filesys` is **not thread-safe**. Add:

```c
struct lock fs_lock;           // defined in pintos/src/userprog/syscall.c, extern in pintos/src/userprog/syscall.h
// syscall_init(): lock_init (&fs_lock);
```

Rules:
- Every call into `filesys_*`, `file_*` happens under `fs_lock`
  (including `load()` in `pintos/src/userprog/process.c`: `filesys_open`, `file_read`, `file_deny_write`,
  and `process_exit`'s `file_close`).
- Validate user memory **before** `lock_acquire` (Phase 2).
- Never hold `fs_lock` across `process_execute` / `process_wait` / `thread_exit`
  (`exec` calls `load()` in the child which needs the lock ⇒ deadlock otherwise).
- `read(fd 0)` / `write(fd 1)` do not need the lock.

## 4. Per-Process File Descriptor Table

Add to `struct thread` in `pintos/src/threads/thread.h` (under `#ifdef USERPROG`):

```c
struct list fd_list;     // list of struct fd_entry
int next_fd;             // starts at 2
```

```c
struct fd_entry { int fd; struct file *file; struct list_elem elem; };
```

| Choice | Array `file *fd[128]` | List (**chosen**) |
|---|---|---|
| `struct thread` size | +512 B (near the < 1 kB guideline, shrinks kernel stack) | +12 B |
| Lookup | O(1) | O(n) (n small) |
| FD limit | hard | none (memory only) |
| Close | free slot | free node |

- In `pintos/src/threads/thread.c` `init_thread`: `list_init (&t->fd_list); t->next_fd = 2;`
  (children list added in Phase 4 here too).
- `fd_alloc (file)`: `malloc` an entry, `fd = next_fd++`, push_back.
  `malloc` failure ⇒ close file, return -1.
- `fd_lookup (fd)`: linear scan; returns NULL for `fd < 2`, unknown, closed.
- `fd_close_all ()` called from `process_exit` in `pintos/src/userprog/process.c` (Phase 4/6).
- fds are **not inherited** by children (`multi-child-fd`): a new thread starts empty.

## 5. `exit` and `halt`

- `sys_halt`: `shutdown_power_off ()` (`pintos/src/devices/shutdown.h`); **no** exit message.
- `sys_exit(status)`: `thread_current ()->exit_code = status; thread_exit ();`
  (message + cleanup done centrally in `process_exit`, Phase 4).
- `exit_bad()`: `sys_exit (-1)`.

## 6. Implementation Steps
1. `pintos/src/userprog/syscall.h`: `extern struct lock fs_lock; void syscall_exit (int status) NO_RETURN;` (if `process.c` needs it).
2. `pintos/src/userprog/syscall.c`: includes (`threads/vaddr.h`, `userprog/pagedir.h`, `threads/synch.h`, `filesys/*`, `devices/shutdown.h`, `devices/input.h`, `lib/kernel/console.h`).
3. Implement validators (Phase 2), dispatcher, `halt`, `exit`, `write(fd==1)` (needed for every test's output).
4. `pintos/src/threads/thread.[ch]`: add fields + init.
5. `pintos/src/userprog/process.c` (`process_exit`): free fd entries (finished in Phase 4).

## 7. Acceptance Criteria
- ✅ `halt` powers off; `exit` prints `name: exit(code)` (after Phase 4 message code).
- ✅ Unknown syscall number or number ≥ `SYS_CLOSE+1` ⇒ exit(-1).
- ✅ `fs_lock` is the only synchronisation protecting the file system.

## 8. Pitfalls
- ⚠️ `f->esp` is a **user** pointer — validate before reading the number.
- ⚠️ Don't forget `f->eax` for value-returning calls.
- ⚠️ `struct thread` must remain small (< 1 kB); keep new fields to pointers/ints/lists.
- ⚠️ `struct fd_entry` memory must be freed in `close` **and** on process exit (leak ⇒ later tests OOM).
- ⚠️ `lock_acquire` panics if the thread already holds the lock (recursive acquire) — keep critical sections flat.

## 9. Design-Doc Material
Data structures: `fd_entry`, `fd_list`, `next_fd`, `fs_lock`. Algorithm:
arg table + dispatch; synchronization: lock hierarchy rule (validate → lock → fs → unlock).
