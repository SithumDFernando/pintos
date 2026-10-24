# Lab 2 / Phase 7 — Testing, Design Document & Submission

**Deadline**: Wed 28 Oct 2026, 11:59 PM — see [Instructions_moodle.txt](../labs/Instructions_moodle.txt)

## 1. Test Strategy

### 1.1 Incremental runs (inside Docker)
```
cd /pintos/src/userprog && make            # build kernel
cd build
make tests/userprog/args-many.result       # single test
make check                                 # full suite (76 tests; long)
```
`make grade` / `make check` prints `pass/FAIL` per test and a summary
(`All 76 tests passed.` target). Screenshot **the summary lines** for Moodle.

### 1.2 Test ladder (matches phase order)
| Gate | Tests | After phase |
|------|-------|-------------|
| G1 | `halt`, `exit` | 3 |
| G2 | `args-*` (5) | 1 (+3 write/exit) |
| G3 | `sc-bad-sp`, `sc-bad-arg`, `bad-*` (4), `*-bad-ptr`, `*-null`, `*-boundary`, `sc-boundary*` | 2 |
| G4 | `create-*`, `open-*`, `close-*`, `read-*`, `write-*` | 5 |
| G5 | `exec-*`, `wait-*`, `multi-recurse`, `multi-child-fd` | 4 |
| G6 | `rox-*` | 6 |
| G7 | full `make check`, repeat 2–3× (flaky races) | all |

### 1.3 Debugging aids
- `pintos -v -k -T 60 --qemu --fs-disk=2 -p tests/userprog/X:X -- -q -f run X`
- `backtrace` tool for kernel panics (`pintos/src/utils/backtrace`).
- GDB: `pintos --gdb`, `pintos-gdb build/kernel.o`, `break syscall_handler`, `break process_exit`.
- Temporary `printf`/`hex_dump` — **remove before submission** (extraneous output fails tests).

### 1.4 Robustness extras (manual)
Not covered by `.ck` but listed in the TODO: oversize cmdline, 100+ open files, many nested execs,
`remove` of running binary, `exec ""`, `wait(-1)`. Write ad-hoc programs under `src/examples/` if desired.

### 1.5 Flakiness checklist
- Race on `ref_cnt` / `children` (Phase 4) → run `multi-recurse`, `exec-multiple` ×10.
- Lock inversion (`fs_lock` + console) → watch for hangs (use `-T` timeout).

## 2. Design Document (`userprog/DESIGNDOC`)

> ⚠️ No template exists in the repository. Recreate the standard Pintos layout:

```
                     +--------------------------+
                     |         CS 140           |
                     | PROJECT 2: USER PROGRAMS |
                     |     DESIGN DOCUMENT      |
                     +--------------------------+
---- GROUP ----        name, index (240159J), email
---- PRELIMINARIES ---- sources consulted
                           ARGUMENT PASSING
---- DATA STRUCTURES ---- (A1)
---- ALGORITHMS ----      (A2) parsing, stack setup, argv order, overflow check
---- RATIONALE ----       (A3) strtok_r vs strtok, separation of kernel/user parsing (A4)
                            SYSTEM CALLS
---- DATA STRUCTURES ---- (B1) child_status, fd_entry, thread fields, fs_lock; (B2) fd association
---- ALGORITHMS ----      (B3) user read/write; (B4) page-table-walk counts for 4096 B and 2 B copies;
                          (B5) wait; (B6) bad-pointer handling without leaks
---- SYNCHRONIZATION ---- (B7) exec load result; (B8) wait/exit races, orphan cases
---- RATIONALE ----       (B9) user memory access approach; (B10) fd table design; (B11) tid↔pid mapping
                     SURVEY QUESTIONS (optional)
```

### 2.1 Answer sources
| Question | Source plan |
|---|---|
| A1–A4 | [Phase 1](./02-lab2-phase1-argument-passing.md) |
| B6, B9 | [Phase 2](./03-lab2-phase2-user-memory-access.md) |
| B1, B2, B10 | [Phase 3](./04-lab2-phase3-syscall-infrastructure.md) |
| B5, B7, B8, B11 | [Phase 4](./05-lab2-phase4-process-control.md) |
| B3, B4 | [Phase 5](./06-lab2-phase5-file-syscalls.md) (+ Phase 2 for page-walk counts) |
| Deny-write section | [Phase 6](./07-lab2-phase6-deny-write.md) |

**B4 hint**: Option A walks once per page spanned: full 4096-byte copy ⇒ min 1 (aligned) / max 2 `pagedir_get_page` calls (+1 if you also check the last byte separately); 2 bytes ⇒ 1–2. Write the exact counts from the final code.

### 2.2 Writing rules
Justify design choices (the Moodle brief asks for justifications); keep consistent with the
final code; include the stack diagram for `args-multiple`.

## 3. Submission Package
1. `pintos/src/userprog/DESIGNDOC` (+ copy to `docs/` if needed).
2. Source: `git diff main -- pintos/src` must contain only intentional changes
   (no debug prints, no build artifacts: `build/` is in `.gitignore`).
3. Screenshots: `make check` summary (+ optionally a few `.result` outputs).
4. Zip/commit per Moodle instructions (verify format with the course page).

## 4. Final Checklist
- [ ] `make clean && make check` all green (76/76)
- [ ] no warnings in `make` output
- [ ] DESIGNDOC complete & matches code
- [ ] screenshots captured
- [ ] plagiarism warning respected: own implementation, own wording

## 5. Open Questions for the Student
1. Do you also need Lab 0/Lab 1 submissions or only Lab 2? (Moodle text says Lab 2 only.)
2. Is Bochs required for Lab 0 screenshots?
3. Submission container format (zip of repo vs patch)?
