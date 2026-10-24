# Pintos Labs — Master Task Checklist (P0 → P2)

> Every actionable task from the lab specifications, broken down into granular sub-tasks.
> Mark items as you go: `[ ]` todo · `[/]` in progress · `[x]` done.

---

## Project 0: Getting Real

> **Weight**: Programming 30% · Documentation 70%
> **Working directory**: `pintos/src/threads`
> **Spec**: [P0_kernal_monitor.md](./P0_kernal_monitor.md)

### 0.1 — Environment Setup & Booting

#### 🐳 Option A: Docker Desktop (Recommended if you don't have native QEMU)

- [ ] Install **Docker Desktop** and ensure it is running
- [ ] Build the Pintos Docker image:
  ```
  docker build -t pintos .
  ```
- [ ] Run the container interactively (mount your source code for live editing):
  ```
  docker run -it --rm -v "%cd%\pintos\src":/pintos/src pintos
  ```
  > On PowerShell use `${PWD}` instead of `%cd%`. On Linux/Mac use `$(pwd)`.
- [ ] **Inside the container**, boot Pintos with QEMU (terminal-only, no GUI window):
  ```
  cd /pintos/src/threads
  make clean && make
  cd build
  pintos -- run alarm-zero
  ```
  - [ ] Verify `Boot complete.` message in terminal output
  - [ ] Take screenshot of terminal output
- [ ] **Inside the container**, boot Pintos with Bochs:
  ```
  cd /pintos/src/threads/build
  pintos --bochs -- run alarm-zero
  ```
  - [ ] Verify `Boot complete.` message in terminal output
  - [ ] Take screenshot of terminal output
- [ ] Place screenshots in `pintos/src/p0/`

> **Note**: Inside Docker there is no graphical window — QEMU/Bochs run in terminal-only mode. Screenshots will be terminal output only (no GUI window screenshots needed).

#### 🖥️ Option B: Native QEMU (if installed locally)

- [ ] Set up Pintos development environment with native toolchain
- [ ] Boot Pintos in **QEMU** (`make qemu` from `threads/`)
  - [ ] Verify `Boot complete.` message in terminal
  - [ ] Verify QEMU graphical window appears (resolve SSH X-forwarding if remote)
  - [ ] Take screenshot of terminal output
  - [ ] Take screenshot of QEMU window
- [ ] Boot Pintos in **Bochs** (`pintos --bochs -- run alarm-zero` from `threads/build/`)
  - [ ] Take screenshot of terminal output
  - [ ] Take screenshot of Bochs window
- [ ] Place all 4 screenshots in `pintos/src/p0/`

### 0.2 — Debugging (GDB Exercises)

> **Docker users**: Run GDB inside the container. Open two terminals into the same container:
> ```
> # Terminal 1 — find your container ID:
> docker ps
> # Terminal 2 — attach to the same container:
> docker exec -it <container_id> bash
> ```
> In Terminal 1: start Pintos with `pintos --gdb -- run alarm-zero`
> In Terminal 2: run `pintos-gdb build/kernel.o`, then `target remote localhost:1234`

#### 0.2.1 — BIOS Tracing
- [ ] Attach GDB to QEMU and trace the very first instruction
  - [ ] 🐳 Docker: use the two-terminal method described above
  - [ ] 🖥️ Native: use `pintos --gdb` in one terminal, GDB in another
- [ ] Answer in DESIGNDOC: *What is the first instruction that gets executed?*
- [ ] Answer in DESIGNDOC: *At which physical address is this instruction located?*
- [ ] Answer in DESIGNDOC: *Can you guess why the first instruction is like this?*
- [ ] Answer in DESIGNDOC: *What are the next three instructions?*

#### 0.2.2 — Bootloader Tracing
- [ ] Set GDB breakpoint at `0x7c00`
- [ ] Trace through `threads/loader.S`, cross-referencing `threads/build/loader.asm`
- [ ] Use `x/i` in GDB to disassemble and compare with source
- [ ] Answer in DESIGNDOC: *How does the bootloader read disk sectors? What BIOS interrupt is used?*
- [ ] Answer in DESIGNDOC: *How does the bootloader decide whether it found the Pintos kernel?*
- [ ] Answer in DESIGNDOC: *What happens when the bootloader could not find the Pintos kernel?*
- [ ] Answer in DESIGNDOC: *At what point does the bootloader transfer control to the Pintos kernel?*

#### 0.2.3 — Kernel Tracing
- [ ] Set breakpoint at `pintos_init()` and trace into C initialization code
- [ ] Read source code of `pintos_init()` in `threads/init.c`
- [ ] Answer in DESIGNDOC: *At the entry of `pintos_init()`, what is the value of `init_page_dir[pd_no(ptov(0))]` in hex?*
- [ ] When `palloc_get_page()` is called for the **first** time:
  - [ ] Answer: *What does the call stack look like?*
  - [ ] Answer: *What is the return value in hex?*
  - [ ] Answer: *What is the value of `init_page_dir[pd_no(ptov(0))]` in hex?*
- [ ] When `palloc_get_page()` is called for the **third** time:
  - [ ] Answer: *What does the call stack look like?*
  - [ ] Answer: *What is the return value in hex?*
  - [ ] Answer: *What is the value of `init_page_dir[pd_no(ptov(0))]` in hex?*

### 0.3 — Kernel Monitor Implementation

- [ ] Locate the TODO comment at **line 136** of `threads/init.c`
- [ ] Implement the kernel monitor shell loop:
  - [ ] Print prompt `CS318> ` and wait for user input
  - [ ] Read a line of input (character by character until newline)
    - [ ] Use `input_getc()` from `devices/input.c`
  - [ ] Parse the input command
  - [ ] If command is `whoami` → print your name
  - [ ] If command is `exit` → break the loop, let kernel finish
  - [ ] For any other input → print `invalid command`
  - [ ] After handling a command, loop back and print `CS318> ` again
- [ ] Relevant helper files to review:
  - [ ] `lib/kernel/console.c` — console output
  - [ ] `lib/stdio.c` — `printf()`
  - [ ] `devices/input.c` — `input_getc()`

### 0.4 — Design Document

- [ ] Copy P0 design document template to `pintos/src/p0/DESIGNDOC`
- [ ] Fill in all GDB tracing answers (sections 0.2.1–0.2.3 above)
- [ ] Fill in kernel monitor design rationale

---

## Project 1: Threads

> **Working directory**: `pintos/src/threads`
> **Build**: `make` in `threads/`, tests run from `threads/build/`
> **Spec**: [P1_threads.md](./P1_threads.md)

### 1.0 — Preparation

- [ ] Read all of: Introduction, Coding Standards, Debugging Tools, Development Tools
- [ ] Skim A.1 Loading through A.5 Memory Allocation (especially A.3 Synchronization)
- [ ] Read B. 4.4BSD Scheduler appendix
- [ ] Trace a context switch in GDB (breakpoint on `schedule()`)
- [ ] Copy P1 design document template to `pintos/src/threads/DESIGNDOC`

### 1.1 — Alarm Clock

> **Goal**: Reimplement `timer_sleep()` to eliminate busy-waiting.
> **Files to modify**: `devices/timer.c`, `threads/thread.c`, `threads/thread.h`

#### 1.1.1 — Design
- [ ] Design a sleeping-threads data structure (sorted list / ordered queue)
- [ ] Decide where to store the wake-up tick for each thread
- [ ] Plan how the timer interrupt handler will check and wake sleeping threads

#### 1.1.2 — Implementation
- [ ] Add a `wake_tick` field (or similar) to `struct thread` in `thread.h`
- [ ] Add a static sleeping-threads list in `timer.c` (or `thread.c`)
- [ ] Modify `timer_sleep()`:
  - [ ] Calculate the absolute wake-up tick (`timer_ticks() + ticks`)
  - [ ] Store it in the current thread's struct
  - [ ] Insert the current thread into the sleeping list (ordered by wake time)
  - [ ] Block the thread (`thread_block()`) with interrupts disabled
- [ ] Modify `timer_interrupt()` handler:
  - [ ] On each tick, check the sleeping list front
  - [ ] Unblock (`thread_unblock()`) any threads whose wake-up tick has arrived
- [ ] Ensure no busy-waiting (no `thread_yield()` loops)

#### 1.1.3 — Testing
- [ ] `alarm-single` — basic single-thread sleep
- [ ] `alarm-multiple` — multiple threads sleeping different durations
- [ ] `alarm-simultaneous` — threads waking at the same time
- [ ] `alarm-priority` — threads wake in priority order
- [ ] `alarm-zero` — `timer_sleep(0)` returns immediately
- [ ] `alarm-negative` — `timer_sleep(-N)` returns immediately

### 1.2 — Priority Scheduling

> **Goal**: Implement strict priority preemptive scheduling and priority donation for locks.
> **Files to modify**: `threads/thread.c`, `threads/thread.h`, `threads/synch.c`, `threads/synch.h`

#### 1.2.1 — Basic Priority Scheduling
- [ ] Modify `thread_unblock()` to insert into `ready_list` in priority order (or sort before picking)
- [ ] Modify `next_thread_to_run()` to always pick the highest-priority thread
- [ ] Modify `thread_create()`: if new thread has higher priority than running → yield immediately
- [ ] Modify `thread_set_priority()`: if lowering priority below another ready thread → yield
- [ ] Modify `thread_yield()`: reinsert current thread in priority order
- [ ] Ensure `sema_up()` wakes the highest-priority waiter
- [ ] Ensure `cond_signal()` wakes the highest-priority waiter
- [ ] Ensure `lock_release()` wakes the highest-priority waiter

#### 1.2.2 — Priority Donation
- [ ] Add donation tracking fields to `struct thread`:
  - [ ] `base_priority` — the thread's own (non-donated) priority
  - [ ] List of donations received (or list of locks held)
- [ ] Add a `holder` field to `struct lock` (track which thread owns the lock)
- [ ] Add a list of waiters to `struct lock` (or use semaphore's waiter list)
- [ ] Implement **single donation**: when thread H tries to acquire lock held by L, donate H's priority to L
- [ ] Implement **multiple donations**: thread L holds multiple locks, takes the max donated priority
- [ ] Implement **nested donation**: if H waits on lock₁ held by M, and M waits on lock₂ held by L, then donate through the chain (depth limit ≤ 8)
- [ ] On `lock_release()`:
  - [ ] Remove the donation from the releasing lock's waiters
  - [ ] Recalculate the thread's effective priority (max of base_priority and remaining donations)
  - [ ] If thread's priority dropped → yield if no longer highest
- [ ] `thread_set_priority()` sets `base_priority`; effective priority = max(base_priority, all donations)
- [ ] `thread_get_priority()` returns effective priority

#### 1.2.3 — Priority Scheduling Tests
- [ ] `priority-change` — thread yields when it lowers its own priority
- [ ] `priority-preempt` — higher-priority thread preempts immediately
- [ ] `priority-fifo` — equal-priority threads run in FIFO / round-robin order
- [ ] `priority-sema` — `sema_up` wakes highest-priority waiter
- [ ] `priority-condvar` — `cond_signal` wakes highest-priority waiter
- [ ] `priority-donate-one` — single donation
- [ ] `priority-donate-multiple` — multiple concurrent donations
- [ ] `priority-donate-multiple2` — additional multiple donation case
- [ ] `priority-donate-nest` — nested donation (H → M → L)
- [ ] `priority-donate-chain` — longer donation chains
- [ ] `priority-donate-sema` — donation interacts with semaphores
- [ ] `priority-donate-lower` — `thread_set_priority()` while receiving donation

### 1.3 — Advanced Scheduler (4.4BSD MLFQS)

> **Goal**: Implement the multi-level feedback queue scheduler with fixed-point arithmetic.
> **Files to modify/create**: `threads/thread.c`, `threads/thread.h`, `threads/fixed-point.h` (new), `devices/timer.c`

#### 1.3.1 — Fixed-Point Arithmetic
- [ ] Create `threads/fixed-point.h` implementing 17.14 fixed-point format
- [ ] Implement conversion: `int_to_fp(n)`, `fp_to_int_zero(x)`, `fp_to_int_nearest(x)`
- [ ] Implement addition: `fp_add(x,y)`, `fp_add_int(x,n)`
- [ ] Implement subtraction: `fp_sub(x,y)`, `fp_sub_int(x,n)`
- [ ] Implement multiplication: `fp_mul(x,y)`, `fp_mul_int(x,n)`
- [ ] Implement division: `fp_div(x,y)`, `fp_div_int(x,n)`
- [ ] Update `Makefile.build` if adding `.c` file (`.h` only doesn't require it)

#### 1.3.2 — Per-Thread MLFQS State
- [ ] Add `nice` field to `struct thread` (range: -20 to 20, default 0)
- [ ] Add `recent_cpu` field to `struct thread` (fixed-point, initial 0)
- [ ] Implement `thread_set_nice(int nice)`:
  - [ ] Set current thread's niceness
  - [ ] Recalculate priority
  - [ ] Yield if no longer highest priority
- [ ] Implement `thread_get_nice()`: return current thread's niceness
- [ ] Implement `thread_get_recent_cpu()`: return `recent_cpu × 100` (nearest integer)
- [ ] Implement `thread_get_load_avg()`: return `load_avg × 100` (nearest integer)

#### 1.3.3 — Global MLFQS State
- [ ] Add global `load_avg` variable (fixed-point, initial 0)
- [ ] Every **1 second** (every `TIMER_FREQ` ticks):
  - [ ] Recalculate `load_avg = (59/60) × load_avg + (1/60) × ready_threads`
  - [ ] `ready_threads` = count of threads that are ready or running (excluding idle)
  - [ ] For **every** thread, recalculate `recent_cpu`:
    - [ ] `recent_cpu = (2 × load_avg)/(2 × load_avg + 1) × recent_cpu + nice`
- [ ] Every **4 ticks**:
  - [ ] Recalculate `priority` for every thread:
    - [ ] `priority = PRI_MAX − (recent_cpu / 4) − (nice × 2)`
    - [ ] Clamp to [PRI_MIN, PRI_MAX]
- [ ] Every tick: increment running thread's `recent_cpu` by 1

#### 1.3.4 — Scheduler Selection
- [ ] Check `thread_mlfqs` flag (set by `-mlfqs` kernel option)
- [ ] When `thread_mlfqs == true`:
  - [ ] Ignore `priority` argument in `thread_create()`
  - [ ] Ignore calls to `thread_set_priority()`
  - [ ] `thread_get_priority()` returns scheduler-calculated priority
  - [ ] Disable priority donation entirely
- [ ] When `thread_mlfqs == false`: use priority scheduler as normal

#### 1.3.5 — MLFQS Tests
- [ ] `mlfqs-load-1` — load average calculation
- [ ] `mlfqs-load-60` — 60 threads load average
- [ ] `mlfqs-load-avg` — load average convergence
- [ ] `mlfqs-recent-1` — recent CPU for single thread
- [ ] `mlfqs-fair-2` — fair scheduling among 2 threads
- [ ] `mlfqs-fair-20` — fair scheduling among 20 threads
- [ ] `mlfqs-nice-2` — niceness affects scheduling
- [ ] `mlfqs-nice-10` — niceness with 10 threads
- [ ] `mlfqs-block` — blocked threads and load average

### 1.4 — Design Document

- [ ] Fill in Alarm Clock section of `threads/DESIGNDOC`
  - [ ] Data structures and their purpose
  - [ ] Algorithm description
  - [ ] Synchronization rationale
  - [ ] Rationale for design choices
- [ ] Fill in Priority Scheduling section
  - [ ] Data structures (donation tracking)
  - [ ] Algorithm (single, multiple, nested donation)
  - [ ] Synchronization
  - [ ] Rationale
- [ ] Fill in Advanced Scheduler section
  - [ ] Data structures (fixed-point, per-thread fields, global state)
  - [ ] Algorithm (formulas, update frequencies)
  - [ ] Ambiguities in spec and your resolution

---

## Project 2: User Programs

> **Working directory**: `pintos/src/userprog`
> **Build**: `make` in `userprog/`, tests run from `userprog/build/`
> **Spec**: [P2_user_programs.md](./P2_user_programs.md)
> **Can build on top of P1 or start fresh** (P1 code not required)

### 2.0 — Preparation

- [ ] Re-read section 1.2.1 Testing (how to run tests)
- [ ] Review `filesys.h` and `file.h` interfaces (don't modify filesys code)
- [ ] Build example user programs: `make` in `src/examples/`
- [ ] Create and format a file system disk:
  ```
  pintos-mkdisk filesys.dsk --filesys-size=2
  pintos -f -q
  ```
- [ ] Practice copying files to the Pintos file system:
  ```
  pintos -p ../../examples/echo -a echo -- -q
  ```
- [ ] Copy P2 design document template to `pintos/src/userprog/DESIGNDOC`

### 2.1 — Process Termination Messages

> **Goal**: Print `"<name>: exit(<status>)\n"` whenever a user process terminates.
> **Files to modify**: `userprog/process.c`, `userprog/syscall.c`

- [ ] On every user process exit (normal or killed), print:
  ```
  printf ("%s: exit(%d)\n", process_name, exit_status);
  ```
- [ ] `process_name` = full name from `process_execute()`, **without** command-line arguments
  - [ ] Parse and store just the program name (first token)
- [ ] Do **NOT** print this message for:
  - [ ] Kernel threads that are not user processes
  - [ ] The `halt` system call
- [ ] Message is optional when a process fails to load
- [ ] Do not print any other extraneous messages

### 2.2 — Argument Passing

> **Goal**: Parse command-line arguments and set up the user stack per x86 cdecl convention.
> **Files to modify**: `userprog/process.c`

#### 2.2.1 — Command-Line Parsing
- [ ] Modify `process_execute()` to extract the program name from the full command line
  - [ ] Use `strtok_r()` to split on spaces
  - [ ] Pass only the program filename to `thread_create()` (not the full command)
- [ ] Pass the full command string to `start_process()` / `load()`

#### 2.2.2 — Stack Setup
- [ ] In `setup_stack()` (or `load()`), after mapping the stack page:
  - [ ] Tokenize the command line into `argv[]` words
  - [ ] Push each argument string onto the stack (right-to-left or any order)
  - [ ] Round stack pointer down to 4-byte alignment (word-align)
  - [ ] Push null sentinel (`argv[argc] = 0`)
  - [ ] Push pointer to each argument string in right-to-left order (`argv[argc-1]` … `argv[0]`)
  - [ ] Push `argv` (pointer to `argv[0]`)
  - [ ] Push `argc`
  - [ ] Push fake return address (`0`)
- [ ] Set `*esp` to the final stack pointer value
- [ ] Validate with `hex_dump()` against the spec's example table (`/bin/ls -l foo bar`)

#### 2.2.3 — Argument Passing Tests
- [ ] `args-none` — no arguments
- [ ] `args-single` — single argument
- [ ] `args-multiple` — multiple arguments
- [ ] `args-many` — many arguments
- [ ] `args-dbl-space` — double spaces between arguments

### 2.3 — User Memory Access

> **Goal**: Safely read/write user-space memory from the kernel without crashing.
> **Files to modify**: `userprog/syscall.c`, optionally `userprog/exception.c`

- [ ] Choose an approach:
  - [ ] **Option A (Verify-then-dereference)**: Check pointer validity before access
    - [ ] Verify pointer is not `NULL`
    - [ ] Verify pointer is below `PHYS_BASE` (use `is_user_vaddr()`)
    - [ ] Verify pointer is mapped (use `pagedir_get_page()`)
  - [ ] **Option B (Page-fault)**: Check only `< PHYS_BASE`, handle invalid access in `page_fault()`
    - [ ] Modify `page_fault()` in `exception.c` to set `eax = 0xffffffff` and copy former value into `eip`
    - [ ] Implement `get_user()` and `put_user()` assembly helpers
- [ ] Terminate the offending process on any invalid pointer (call `thread_exit()` with status -1)
- [ ] Handle edge cases:
  - [ ] Pointer to kernel memory (`>= PHYS_BASE`)
  - [ ] Null pointer
  - [ ] Unmapped page
  - [ ] Buffer spanning valid and invalid pages
- [ ] Ensure no resource leaks on bad pointer (release locks, free memory)

### 2.4 — System Call Infrastructure

> **Goal**: Read the syscall number from the user stack and dispatch to the correct handler.
> **Files to modify**: `userprog/syscall.c`, `userprog/syscall.h`

- [ ] In `syscall_handler()`:
  - [ ] Validate the stack pointer (`intr_frame->esp`)
  - [ ] Read the system call number from `esp`
  - [ ] Read arguments from `esp + 4`, `esp + 8`, `esp + 12` (depending on syscall)
  - [ ] Validate each pointer argument before dereferencing
  - [ ] Dispatch to the appropriate handler function
  - [ ] Store return value in `intr_frame->eax`
- [ ] Design a file descriptor table for each process:
  - [ ] Reserve fd 0 for `STDIN_FILENO` and fd 1 for `STDOUT_FILENO`
  - [ ] Allocate new fds starting from 2
  - [ ] Store per-process (in `struct thread` or a child struct)
  - [ ] Each `open()` returns a unique fd; fds are not inherited by children
- [ ] Add a global file system lock to serialize all file system operations

### 2.5 — System Call Implementations

> **Files to modify**: `userprog/syscall.c`, `threads/thread.c`, `threads/thread.h`, `userprog/process.c`

#### 2.5.1 — `halt`
- [ ] Call `shutdown_power_off()` (from `devices/shutdown.h`)

#### 2.5.2 — `exit`
- [ ] Store the exit status for the parent to retrieve
- [ ] Print termination message: `printf("%s: exit(%d)\n", ...)`
- [ ] Close all open file descriptors
- [ ] Close the executable file (re-enable writes)
- [ ] Call `thread_exit()`

#### 2.5.3 — `exec`
- [ ] Validate the `cmd_line` pointer
- [ ] Call `process_execute(cmd_line)`
- [ ] Wait for the child to finish loading before returning
  - [ ] Use a semaphore to synchronize with the child's `load()` result
- [ ] Return the child's `pid`, or `-1` if loading failed

#### 2.5.4 — `wait`
- [ ] Implement `process_wait(pid)`:
  - [ ] Verify `pid` is a direct child of the calling process
  - [ ] If child is still alive → block until it exits
  - [ ] Retrieve and return the child's exit status
  - [ ] Return `-1` if:
    - [ ] `pid` is not a direct child
    - [ ] `wait` has already been called on this `pid`
    - [ ] Child was killed by the kernel
  - [ ] Free child's resources after wait completes
- [ ] Design parent-child tracking data structure:
  - [ ] Each thread maintains a list of its children
  - [ ] Child struct stores: pid, exit status, alive/dead flag, wait semaphore
  - [ ] Handle case where parent exits before child (orphans)
  - [ ] Handle case where child exits before parent calls `wait`

#### 2.5.5 — `create`
- [ ] Validate `file` pointer
- [ ] Call `filesys_create(file, initial_size)` (acquire file system lock)
- [ ] Return `true` on success, `false` on failure
- [ ] Reject null filename (terminate process or return false)

#### 2.5.6 — `remove`
- [ ] Validate `file` pointer
- [ ] Call `filesys_remove(file)` (acquire file system lock)
- [ ] Return `true` on success, `false` on failure

#### 2.5.7 — `open`
- [ ] Validate `file` pointer
- [ ] Call `filesys_open(file)` (acquire file system lock)
- [ ] If `NULL` returned → return `-1`
- [ ] Allocate a new file descriptor (≥ 2)
- [ ] Store the `struct file *` in the process's fd table
- [ ] Return the fd number

#### 2.5.8 — `filesize`
- [ ] Look up `fd` in the process's fd table
- [ ] If invalid fd → return `-1` or terminate
- [ ] Call `file_length(file)` and return result

#### 2.5.9 — `read`
- [ ] Validate the `buffer` pointer (entire range: `buffer` to `buffer + size`)
- [ ] If `fd == 0` (stdin): read using `input_getc()` in a loop
- [ ] If `fd == 1`: return `-1` (can't read from stdout)
- [ ] Otherwise:
  - [ ] Look up fd in the process's fd table
  - [ ] Call `file_read(file, buffer, size)` (acquire file system lock)
  - [ ] Return bytes read, or `-1` on error

#### 2.5.10 — `write`
- [ ] Validate the `buffer` pointer (entire range)
- [ ] If `fd == 1` (stdout): call `putbuf(buffer, size)` — write all at once for small sizes
- [ ] If `fd == 0`: return `-1` (can't write to stdin)
- [ ] Otherwise:
  - [ ] Look up fd in the process's fd table
  - [ ] Check if file has writes denied → return 0
  - [ ] Call `file_write(file, buffer, size)` (acquire file system lock)
  - [ ] Return bytes written

#### 2.5.11 — `seek`
- [ ] Look up `fd` in the process's fd table
- [ ] Call `file_seek(file, position)`

#### 2.5.12 — `tell`
- [ ] Look up `fd` in the process's fd table
- [ ] Call `file_tell(file)` and return result

#### 2.5.13 — `close`
- [ ] Look up `fd` in the process's fd table
- [ ] Call `file_close(file)` (acquire file system lock)
- [ ] Remove the fd from the process's table (invalidate it)

### 2.6 — Denying Writes to Executables

> **Goal**: Prevent writing to a file while it is being run as a process's executable.
> **Files to modify**: `userprog/process.c`

- [ ] After successfully loading an executable in `load()`:
  - [ ] Keep the executable file open (store `struct file *` in the thread struct)
  - [ ] Call `file_deny_write()` on the executable file
- [ ] When the process exits:
  - [ ] Call `file_close()` on the executable file (automatically re-enables writes)
- [ ] Add a `struct file *executable` field to `struct thread`

### 2.7 — Temporary `process_wait()` Fix

> **Note**: This is an early bootstrap step before full `wait` is implemented.

- [ ] Initially change `process_wait()` to an infinite loop (`while(1) thread_yield()` or `sema_down`)
  - [ ] This prevents Pintos from shutting down before processes finish
- [ ] Replace with proper implementation once `wait` syscall is done (§2.5.4)

### 2.8 — Robustness & Edge Cases

> **Goal**: Pintos must be bulletproof — nothing a user program does should crash the kernel.

- [ ] Bad pointer: null, kernel address, unmapped address → terminate process with exit(-1)
- [ ] Bad buffer: partially valid buffer spanning page boundary
- [ ] Bad fd: invalid file descriptor number → return error or terminate
- [ ] Bad syscall number: unrecognized syscall number → terminate process
- [ ] Stack overflow: esp below user stack page
- [ ] Bad string: unterminated string crossing into kernel memory
- [ ] Empty filename: `open("")`, `create("")`, `remove("")`
- [ ] Self-removal: remove the currently running executable
- [ ] Double-close: closing an fd that's already closed
- [ ] Concurrent file system access: ensure global lock prevents races

### 2.9 — System Call Tests

> All tests are in `tests/userprog/`.

#### Argument Passing
- [ ] `args-none`
- [ ] `args-single`
- [ ] `args-multiple`
- [ ] `args-many`
- [ ] `args-dbl-space`

#### Basic Syscalls
- [ ] `halt`
- [ ] `exit`
- [ ] `create-normal`
- [ ] `create-empty`
- [ ] `create-null`
- [ ] `create-bad-ptr`
- [ ] `create-long`
- [ ] `create-exists`
- [ ] `create-bound`
- [ ] `open-normal`
- [ ] `open-missing`
- [ ] `open-boundary`
- [ ] `open-empty`
- [ ] `open-null`
- [ ] `open-bad-ptr`
- [ ] `open-twice`
- [ ] `close-normal`
- [ ] `close-twice`
- [ ] `close-stdin`
- [ ] `close-stdout`
- [ ] `close-bad-fd`
- [ ] `read-normal`
- [ ] `read-bad-ptr`
- [ ] `read-boundary`
- [ ] `read-zero`
- [ ] `read-stdout`
- [ ] `read-bad-fd`
- [ ] `write-normal`
- [ ] `write-bad-ptr`
- [ ] `write-boundary`
- [ ] `write-zero`
- [ ] `write-stdin`
- [ ] `write-bad-fd`

#### Process Control
- [ ] `exec-once`
- [ ] `exec-arg`
- [ ] `exec-multiple`
- [ ] `exec-missing`
- [ ] `exec-bad-ptr`
- [ ] `wait-simple`
- [ ] `wait-twice`
- [ ] `wait-killed`
- [ ] `wait-bad-pid`
- [ ] `multi-recurse`
- [ ] `multi-child-fd`

#### Robustness
- [ ] `bad-read`
- [ ] `bad-write`
- [ ] `bad-read2`
- [ ] `bad-write2`
- [ ] `bad-jump`
- [ ] `bad-jump2`

#### Corner Cases
- [ ] `sc-bad-sp`
- [ ] `sc-bad-arg`
- [ ] `sc-boundary`
- [ ] `sc-boundary-2`
- [ ] `sc-boundary-3`

#### Resource Management
- [ ] `multi-oom` — processes spawned until out of memory; all must exit cleanly
- [ ] `rox-simple` — deny writes to running executable
- [ ] `rox-child` — deny writes to child's executable
- [ ] `rox-multichild` — deny writes across multiple child processes

### 2.10 — Design Document

- [ ] Fill in Argument Passing section of `userprog/DESIGNDOC`
  - [ ] Data structures
  - [ ] Stack setup algorithm and diagram
  - [ ] Rationale for design choices
- [ ] Fill in System Calls section
  - [ ] Data structures (fd table, parent-child tracking)
  - [ ] Algorithm (dispatch, user memory access, wait/exec synchronization)
  - [ ] Synchronization (file system lock, parent-child semaphores)
  - [ ] Rationale
- [ ] Fill in Denying Writes to Executables section

---

## Quick Reference — Files Modified per Project

| File | P0 | P1 | P2 |
|---|:---:|:---:|:---:|
| `threads/init.c` | ✏️ | | |
| `devices/timer.c` | | ✏️ | |
| `threads/thread.c` | | ✏️ | ✏️ |
| `threads/thread.h` | | ✏️ | ✏️ |
| `threads/synch.c` | | ✏️ | |
| `threads/synch.h` | | ✏️ | |
| `threads/fixed-point.h` | | 🆕 | |
| `userprog/process.c` | | | ✏️ |
| `userprog/syscall.c` | | | ✏️ |
| `userprog/syscall.h` | | | ✏️ |
| `userprog/exception.c` | | | ✏️ (maybe) |

✏️ = Modified · 🆕 = New file
