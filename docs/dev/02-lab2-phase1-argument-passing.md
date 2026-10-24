# Lab 2 / Phase 1 — Argument Passing

**Spec**: [P2 §4.3.3–4.5](../labs/P2_user_programs.md) · **Files**: `userprog/process.c`
**Tests**: `args-none`, `args-single`, `args-multiple`, `args-many`, `args-dbl-space`
**Depends on**: minimal `exit` + `write(1)` (Phase 3) and a bootstrap `process_wait` (Phase 4)

## 1. Current State (verified in `process.c`)

| Function | Today | Problem |
|----------|-------|---------|
| `process_execute(file_name)` | copies cmdline to a page; `thread_create(file_name, …)` | thread name = full cmdline (`"ls -l foo"`), truncated to 15 chars |
| `start_process(file_name_)` | `load(file_name,…)`, then frees the page | cmdline consumed by `load` |
| `load(file_name,…)` | `filesys_open(file_name)` | opens `"args-single onearg"` → fails |
| `setup_stack(esp)` | maps page, `*esp = PHYS_BASE` | no argv/argc |

## 2. Design

### 2.1 Split program name from arguments
- In `process_execute`: make a **second** copy (local `char name[16]`-sized is
  too small for parsing; use a small `palloc` page or `strlcpy` into a
  `char prog[NAME_MAX+1]` on stack — `NAME_MAX` is 14, so stack is safe).
  `strtok_r(copy, " ", &save)` → first token = program name.
- If the cmdline is empty / only spaces → first token `NULL` → return `TID_ERROR`.
- `thread_create(prog_name, PRI_DEFAULT, start_process, fn_copy)` — thread
  name is now just the program (needed for `"%s: exit(%d)"`, Phase 4).
- `fn_copy` keeps the **full** command line; `load()` receives it.

### 2.2 `load()` takes the full cmdline
- Parse program name again (own `strtok_r` on a local copy of ≤ 14+1 chars —
  reject if first token longer than `NAME_MAX`; filesys would fail anyway).
- `filesys_open(prog_name)`; error text must stay
  `load: %s: open failed` with the **program name only**
  (`exec-missing.ck` expects `load: no-such-file: open failed`).
- Pass the full cmdline to `setup_stack(esp, cmdline)`.

### 2.3 `setup_stack(esp, cmdline)` — x86 cdecl layout

Target layout for `/bin/ls -l foo bar` (spec example), growing **down**:

```
PHYS_BASE
 "bar\0" "foo\0" "-l\0" "/bin/ls\0"   (strings, any order)
 padding to 4-byte boundary (word-align, 0 value bytes)
 argv[argc] = NULL
 argv[argc-1] … argv[0]               (pointers to the strings above)
 argv        (char **, = address of argv[0])
 argc
 0           (fake return address)  <- *esp
```

Algorithm:
1. Make a private writable copy of the cmdline (palloc page; it is already
   `fn_copy`, but `load()` only has `const char *` — copy again or change
   signature to pass a mutable buffer).
2. **Pass 1** – `strtok_r` to count `argc` and total string bytes
   (`sum(strlen+1)`).
3. **Overflow check**: total = strings + align(≤3) + `4*(argc+1)` + 4 (argv)
   + 4 (argc) + 4 (ret) must be `< PGSIZE`; otherwise `return false` (load fails
   cleanly — *a user cannot crash the kernel with a huge cmdline*).
4. **Pass 2** – `sp = PHYS_BASE`; for each token (in order):
   `sp -= len+1; memcpy(sp, tok, len+1); argv_addr[i] = sp;`
   (`argv_addr[]` must be heap/page memory, not a big stack array — kernel
   stack is < 4 KB. Allocate with `palloc_get_page` and free before return.)
5. `sp = (uint8_t*)((uintptr_t)sp & ~3)` (word-align; zero padding already
   guaranteed because the page was `PAL_ZERO`).
6. Push words (as `uint32_t*`, pre-decrement):
   `0` (argv[argc]), `argv_addr[argc-1..0]`, `argv` (= current `sp`),
   `argc`, `0`.
7. `*esp = sp`.

Because the kernel writes through **kernel virtual address of the page**
(`kpage`) *or* through user address with the user page directory active:
`load()` already ran `process_activate()`, so writing via the user virtual
address is valid. Writing through `kpage + (user_addr - (PHYS_BASE-PGSIZE))`
is the safer, explicit option — recommended to avoid depending on activation.

### 2.4 Failure paths / resource ownership
- `start_process` frees `fn_copy` after `load` returns (already). All
  temporary pages allocated in `setup_stack` must be freed on every path.
- If tokenization finds `argc == 0` → fail.

## 3. Implementation Steps

1. Edit `process_execute` (name split, `TID_ERROR` on empty).
2. Change `load()` / `setup_stack()` prototypes (`static` ⇒ safe to change).
3. Implement helper `static bool push_arguments (void **esp, uint8_t *kpage, const char *cmdline)`.
4. Temporarily `hex_dump ((uintptr_t) *esp, *esp, PHYS_BASE - *esp, true);`
   (from `<stdio.h>`) to compare with the spec table; **remove before submission**.
5. Run with `pintos -v -k --fs-disk=2 -p tests/userprog/args-many:args-many -- -q run 'args-many a b c'`
   (adapt paths) and compare `argc`/`argv` printed by `args.c`.

## 4. Acceptance Criteria
- ✅ `args-none` … `args-dbl-space` pass.
- ✅ Double spaces collapse (`strtok_r` skips empty tokens).
- ✅ `*esp` is 4-byte aligned; `argv[argc] == NULL`.
- ✅ No hex_dump / debug printf left.

## 5. Pitfalls
- ⚠️ `strtok_r` mutates its input → never tokenize the `const` original.
- ⚠️ Do not use `strtok` (non-reentrant, not available in Pintos).
- ⚠️ Pushing pointers *before* the strings are placed gives wrong addresses.
- ⚠️ `argv` pointer value is the address where `argv[0]` is stored, not `argv[0]`.
- ⚠️ Thread `name[16]`: program names > 15 chars are truncated (all provided tests are short).

## 6. Design-Doc Material
Data structures: none new (maybe `argv_addr` temp array). Algorithm: the 7
steps above with a stack diagram. Rationale: two-pass to enforce the page
limit *before* mutating the stack; separate copies avoid `strtok_r` races.
