# Lab 2 / Phase 2 — Safe User Memory Access

**Files**: `userprog/syscall.c` (+ optionally `userprog/exception.c`)
**Tests**: `sc-bad-sp`, `sc-bad-arg`, `sc-boundary(-2,-3)`, `bad-read(2)`, `bad-write(2)`,
`bad-jump(2)`, `create-bad-ptr`, `open-bad-ptr`, `read/write-bad-ptr`, `exec-bad-ptr`,
`exec-bound(-2,-3)`, `create-bound`, `open-boundary`, `read/write-boundary`, `*-null`
**Rule**: *No user program may crash the kernel; invalid access ⇒ terminate with exit(-1).*

## 1. Approach Decision

| | A. Verify-then-dereference | B. Fault-based (`get_user`/`put_user`) |
|---|---|---|
| Speed | slower (page-table walk per check) | fastest (MMU does it) |
| Complexity | low, all in `syscall.c` | needs asm helpers + `page_fault()` edit |
| Risk | must check every byte range | must release resources on fault |
| Debuggability | easy | harder |

**Choice: A** (clearer for the design doc and for 76 tests). Option B noted as
future optimisation.

## 2. Validity Predicate

A byte address `p` is valid iff
`p != NULL && is_user_vaddr(p) && pagedir_get_page(thread_current()->pagedir, p) != NULL`.

(`vaddr.h`: `is_user_vaddr(v) = v < PHYS_BASE`; `pagedir_get_page` returns NULL when unmapped.)

## 3. Helper API (all `static` in `syscall.c`)

```c
static bool valid_byte   (const void *p);                     // predicate above
static bool valid_range  (const void *p, size_t n);           // all pages touched
static bool valid_string (const char *s);                     // up to & incl. NUL
static bool read_word    (const void *uaddr, uint32_t *out);  // 4 bytes, copy-in
static void exit_bad (void) NO_RETURN;                        // sys_exit(-1)
```

### `valid_range(p, n)`
- `n == 0` → `true` (but pointer still need not be validated? `read-zero`/`write-zero`
  pass a valid ptr; being strict is also fine — document choice).
- Check `p` and `p + n - 1` (overflow guard: `p + n < p` ⇒ invalid), and **one
  byte per page in between**: `for (a = pg_round_down(p) + PGSIZE; a < p + n; a += PGSIZE)`.
- Both ends must be user addresses: `boundary` tests place buffers across a
  mapped/unmapped page edge and across `PHYS_BASE`.

### `valid_string(s)`
- Loop char-by-char; call `valid_byte` whenever the address is at a page start
  (or simply every byte — simple & correct, strings are short). Stop at `'\0'`.
- Bound the scan (e.g. `PGSIZE`) to avoid scanning forever on an unterminated
  string that stays in mapped memory (`create-long` uses long names; filesys limit 14).

### `read_word` / syscall args
- Stack args for `SYS_x` live at `esp`, `esp+4`, `esp+8`, `esp+12`.
- **Validate `esp .. esp + 4*(1+nargs) - 1`** *before* reading anything — this is
  exactly `sc-bad-arg` (syscall number at `0xbffffffc`, args beyond `PHYS_BASE`)
  and `sc-bad-sp` (`esp` far below stack).
- Copy args into a local `uint32_t args[4]` (kernel stack) then work only on the copy.

## 4. Where Checks Happen (per syscall)

| Syscall | Checks before use |
|---------|-------------------|
| `halt` | stack words (just the number) |
| `exit(status)` | `esp` + 1 arg |
| `exec(cmd)` | `cmd` string (`valid_string`) |
| `wait(pid)` | arg word |
| `create(file,size)` / `open(file)` / `remove(file)` | `file` string |
| `read(fd,buf,n)` | `valid_range(buf,n)` (user buffer is **written**) |
| `write(fd,buf,n)` | `valid_range(buf,n)` (user buffer is **read**) |
| `filesize/seek/tell/close` | arg words only |

**Order matters**: validate **before** taking `fs_lock` (Phase 3) → nothing to
release when a process is killed (no leak, no deadlock).

## 5. Page-Fault Path (`exception.c`)
User-mode faults (e.g. `bad-read`: `*(int*)NULL`) reach `page_fault()` →
`kill()` → `thread_exit()`. Required changes:
- Set the dying process's exit status to **-1** before `thread_exit()` (Phase 4's
  `cur->exit_code = -1`; since default is -1 and `exit()` overwrites it, kernel
  kills need no code change except ensuring the default remains -1).
- Optional: drop the verbose `Page fault at …` printf for user faults;
  tests use `IGNORE_USER_FAULTS` so the output is tolerated either way.
- Kernel-mode fault in the middle of a syscall must **not** happen with
  Option A (validation first). Leave `PANIC` in `kill()` as an assertion.

## 6. Edge Cases Checklist
- [ ] `NULL` pointers (`create-null`, `open-null`, `exec` null).
- [ ] pointer ≥ `PHYS_BASE` (`bad-read2`, `bad-write2`, kernel address).
- [ ] unmapped user page.
- [ ] string/buffer spanning valid→invalid page (`*-boundary`, `exec-bound-*`).
- [ ] `esp` itself invalid (`sc-bad-sp`), `esp` valid but args cross `PHYS_BASE` (`sc-bad-arg`).
- [ ] jumping to kernel address (`bad-jump`, `bad-jump2`): fault arrives via
  `page_fault` with `eip` in kernel space → still `kill()` (user code seg) ⇒ exit(-1).
- [ ] length overflow `p + n` wrapping.

## 7. Acceptance Criteria
- ✅ All listed tests pass, no kernel panic/assert in any of them.
- ✅ Every dereference of a user pointer in kernel code is preceded by a check.

## 8. Design-Doc Material
Describe Option A, predicate, range walk by page, validation-before-lock rule,
and why Option B was rejected (complexity vs. benefit).
