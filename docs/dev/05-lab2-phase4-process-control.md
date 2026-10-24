# Lab 2 / Phase 4 — Process Control: exec, wait, exit

**Files**: `userprog/process.[ch]`, `threads/thread.[ch]`, `userprog/syscall.c`
**Tests**: `exec-once/arg/bound*/multiple/missing/bad-ptr`, `wait-simple/twice/killed/bad-pid`,
`multi-recurse`, `multi-child-fd`, termination message in **every** test.

## 1. Bootstrap (do this first)
`process_wait()` currently returns `-1` ⇒ `run_task()` returns and Pintos
shuts down before the user program runs. Temporary fix to unblock Phase 1:
`process_wait` → `while (true) thread_yield ();` — replaced by the real design below.

## 2. Requirements Recap
- `exec(cmd)` returns pid **only after** the child finished `load()`; `-1` if load fails.
- `wait(pid)`: returns child exit code; `-1` if `pid` is not a **direct child**,
  already waited, or the child was **killed by the kernel**.
- Print `"%s: exit(%d)\n"` exactly once per user process exit (not for `halt`,
  not for kernel threads); name = program name without args.
- Parent may exit first (orphans) or child may exit first (zombie status must survive).

## 3. Data Structure: shared `struct child_status`

A thread's memory page is freed when it dies, so the exit code cannot live in
`struct thread` if the parent reads it later. Use a **heap record shared by both**,
reference-counted:

```c
struct child_status {
  tid_t tid;
  int exit_code;              // default -1 (killed by kernel)
  bool load_ok;               // set by child before load_sema up
  struct semaphore load_sema; // child -> parent: "load finished"
  struct semaphore exit_sema; // child -> parent: "I exited"
  int ref_cnt;                // 2 initially (parent + child)
  struct lock ref_lock;
  struct list_elem elem;      // in parent's children list
};
```

`struct thread` additions (`#ifdef USERPROG`):

```c
int exit_code;                    // status to publish (default -1)
struct list children;             // list of child_status owned by this parent
struct child_status *my_status;   // record shared with my parent (NULL for initial/kernel threads)
struct file *executable;          // Phase 6
struct list fd_list; int next_fd; // Phase 3
```
`init_thread`: `list_init(&t->children); t->my_status = NULL; t->exit_code = -1; t->executable = NULL;`

**Release helper** (`static void release_status (struct child_status *cs)`):
`lock_acquire; n = --cs->ref_cnt; lock_release; if (n == 0) free (cs);`
(A malloc'd lock is gone with the struct; release before free to avoid use-after-free.)

## 4. `process_execute(cmdline)` (extends Phase 1)

```
cs = malloc(*cs); init sems(0), lock, ref_cnt=2, exit_code=-1, load_ok=false
args = { fn_copy, cs }     // pass both: use a small malloc'd struct or place cs pointer
tid = thread_create (prog_name, PRI_DEFAULT, start_process, args)
if tid == TID_ERROR: free fn_copy, args, cs; return TID_ERROR
cs->tid = tid; list_push_back (&cur->children, &cs->elem)
sema_down (&cs->load_sema)               // wait for child load
if (!cs->load_ok) { list_remove; release_status; (its child ref released by child) return TID_ERROR; }
return tid
```

⚠️ `cs->tid` must be set before the parent reads it, but the **child must not read
`cs->tid`** (it can start running first) — child uses `thread_current()->tid`.
⚠️ Do not hold `fs_lock` here (child's `load` needs it).

## 5. `start_process(args)`
```
t->my_status = cs
success = load (cmdline, &eip, &esp)       // under fs_lock for fs calls
free fn_copy / args
cs->load_ok = success
sema_up (&cs->load_sema)                   // ALWAYS, success or failure
if (!success) thread_exit ();              // process_exit publishes -1
jump to user mode via intr_exit
```

## 6. `process_exit()` — single cleanup point (order matters)

```
cur = thread_current ()
if (cur->pagedir != NULL)  // a user process
    printf ("%s: exit(%d)\n", cur->name, cur->exit_code);        // 1
close all fds (under fs_lock)                                    // 2
if (cur->executable) { lock; file_close; unlock }                // 3  (re-enables writes — Phase 6)
for each child_status in cur->children: list_remove; release_status   // 4  (orphan adoption: just drop ref)
if (cur->my_status) { cs->exit_code = cur->exit_code; sema_up (&cs->exit_sema); release_status (cs) }  // 5  last!
destroy page directory (existing code)                           // 6
```

⚠️ **Step 3 must precede step 5** — `rox-child` expects the parent, once `wait`
returns, to be able to write the executable again.
⚠️ Print the message **before** destroying `pagedir` and before waking the parent
(output ordering in `.ck` files, e.g. `wait-killed`: `child-bad: exit(-1)` then parent line).
⚠️ Print only if user process; kernel threads (idle, main, test threads) have
`pagedir == NULL`; load failure prints (accepted by `exec-missing.ck`).
⚠️ Kernel kill (`exception.c`): `exit_code` default is -1 ⇒ `wait` returns -1 ⇒ `wait-killed` ✅.

## 7. `process_wait(child_tid)`
```
find cs in cur->children with cs->tid == child_tid
if none -> return -1                        // not a child / already waited / bad pid
sema_down (&cs->exit_sema)                  // blocks only if still alive
code = cs->exit_code
list_remove (&cs->elem); release_status (cs)   // removing ⇒ second wait returns -1
return code
```
- `wait-twice`: second call finds nothing ⇒ -1 ✅. `wait-bad-pid`: pid not in list ⇒ -1 ✅.
- Child already dead: semaphore already upped ⇒ `sema_down` returns immediately ✅.
- Initial thread's `children` list must be initialised (it is `init_thread`'d in `thread_init`).

## 8. `exec` / `wait` Syscalls (in `syscall.c`)
- `sys_exec(cmd)`: `valid_string(cmd)` else exit(-1); `return process_execute (cmd)` (`TID_ERROR` = -1 ✅ same value as spec's `-1`).
- `sys_wait(pid)`: `return process_wait (pid)`.
- `exec-bound*`: string spans page boundary ⇒ handled by Phase 2 `valid_string`.

## 9. Edge Cases & Failure Modes
| Case | Handling |
|------|---------|
| `thread_create` fails | free fn_copy/args/cs; `TID_ERROR` |
| load fails | child ups `load_sema`, exits −1; parent removes record, returns −1 |
| parent exits before child | parent drops its ref; child later drops its ref → freed; no leak |
| child exits before parent waits | `exit_sema` already 1; record kept (ref_cnt 1) until `wait` or parent exit |
| user passes huge/odd cmdline | Phase 1 size check ⇒ load fails ⇒ -1 |
| grandchild wait by grandparent | not in list ⇒ -1 |
| `multi-recurse` (depth 15, each parent waits) | works because every parent waits only its direct child |
| out of memory (malloc fail) in exec | `TID_ERROR` |

## 10. Verification
`exec-*`, `wait-*`, `multi-recurse`, `multi-child-fd`; confirm no leaks by running
the whole suite twice in one boot? (not supported) — instead add a temporary
`palloc` free-page counter print at shutdown and compare before/after a loop of `exec`s.

## 11. Pitfalls
- ⚠️ `sema_down` inside `process_execute` while holding **no** locks.
- ⚠️ Never free `cs` on the parent path without checking `ref_cnt` (child may still run).
- ⚠️ Message uses `thread_name()` (≤ 15 chars); program names in tests are short.
- ⚠️ `process_exit` can be reached with `cur->my_status == NULL` (initial thread) — guard.

## 12. Design-Doc Material
Structures: `child_status`, new `thread` fields. Algorithm: execute/wait/exit sequences
(two semaphores) + ref-count lifetime. Sync: semaphores for ordering, per-record
lock for refcount. Rationale: heap record decouples the zombie status from the dead thread's page.
