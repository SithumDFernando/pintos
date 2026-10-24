# Lab 1 — Threads: Alarm Clock, Priority Scheduling/Donation, MLFQS

**Spec**: [P1_threads.md](../labs/P1_threads.md)
**Files**: `pintos/src/devices/timer.c`, `pintos/src/threads/thread.[ch]`, `pintos/src/threads/synch.[ch]`, `pintos/src/threads/fixed-point.h` (new)
**Tests**: 27 in `pintos/src/tests/threads` (`make check` from `pintos/src/threads/` in container)

> Baseline (verified): `timer_sleep()` busy-waits with `thread_yield()`;
> `thread_unblock()` does `list_push_back(&ready_list)`; `timer_interrupt()`
> only does `ticks++; thread_tick();`.

## Phase 1 — Alarm Clock (6 tests)

**Data**: add `int64_t wake_tick;` to `struct thread`; a static
`struct list sleep_list` in `pintos/src/devices/timer.c` ordered by `wake_tick`; the thread's
`elem` is reused for the sleep list (safe: a sleeping thread is blocked).

**`timer_sleep(ticks)`**
1. `if (ticks <= 0) return;` (covers `alarm-zero`, `alarm-negative`).
2. `old = intr_disable(); cur->wake_tick = timer_ticks()+ticks;`
   `list_insert_ordered(&sleep_list,&cur->elem,wake_less,NULL);`
   `thread_block(); intr_set_level(old);`

**`timer_interrupt()`** after `ticks++`: while the list head has
`wake_tick <= ticks`: `list_pop_front` → `thread_unblock`. (Interrupt context:
no blocking calls; `thread_unblock` is allowed.)

⚠️ Do **not** call `thread_yield()` loops. ⚠️ Sorted insert must be stable for
equal ticks (`alarm-simultaneous`) — use `<` not `<=` in comparator.
🔬 `alarm-single/multiple/simultaneous/priority/zero/negative`.

## Phase 2 — Priority Scheduling (5 tests)

- Add `ready_list` ordering: `thread_unblock` and `thread_yield` use
  `list_insert_ordered` by priority **descending**, stable (FIFO within equal).
  Alternative: keep push_back and `list_max` in `next_thread_to_run` (O(n) pick,
  O(1) insert) — pick one and justify.
- Preemption points: end of `thread_create` (new > current → yield),
  `thread_set_priority` (lowered below top ready → yield), `sema_up`
  (woken thread higher → yield; **not** inside interrupt context — use
  `intr_yield_on_return` if `intr_context()`).
- `sema_up`: wake highest-priority waiter (`list_max`/sort of `sema->waiters`).
- `cond_signal`: pick the waiter whose semaphore's top waiter has highest priority.
🔬 `priority-change/preempt/fifo/sema/condvar`.

## Phase 3 — Priority Donation (7 tests)

Add to `struct thread`: `int base_priority; struct list held_locks;
struct lock *waiting_on;`; to `struct lock`: `int max_priority;
struct list_elem elem;` (or recompute from `semaphore.waiters`).

- `lock_acquire`: if holder exists and holder.priority < cur.priority:
  set `cur->waiting_on = lock`, then donate up the chain
  (`while (l && depth++ < 8) { l->holder->priority = max(...); l = l->holder->waiting_on; }`),
  then `sema_down`; after acquiring: `waiting_on = NULL`, insert lock into `held_locks`.
- `lock_release`: remove from `held_locks`; recompute
  `priority = max(base_priority, max over held_locks of top waiter priority)`;
  `sema_up` then yield if needed.
- `thread_set_priority`: set `base_priority`; effective = max(base, donations); yield if no longer highest.
- Disable donation when `thread_mlfqs`.
⚠️ Interrupts must be disabled around donation updates (shared with timer ISR scheduling).
🔬 `priority-donate-one/multiple/multiple2/nest/sema/lower/chain`.

## Phase 4 — MLFQS (9 tests)

1. `pintos/src/threads/fixed-point.h` — 17.14 format, `#define F (1<<14)`: `int_to_fp`,
   `fp_to_int_zero`, `fp_to_int_nearest`, add/sub/mul/div (use `int64_t`
   intermediate for mul/div).
2. `struct thread`: `int nice; fixed recent_cpu;` ; global `fixed load_avg`.
3. In `timer_interrupt` (when `thread_mlfqs`):
   - every tick: `recent_cpu++` for running (not idle);
   - every `TIMER_FREQ`: `load_avg = 59/60*load_avg + 1/60*ready_threads`,
     then `recent_cpu = (2*la)/(2*la+1)*recent_cpu + nice` for all threads (`thread_foreach`);
   - every 4 ticks: `priority = PRI_MAX - recent_cpu/4 - 2*nice` clamped to [0,63]; re-sort ready list.
4. `thread_set_nice/get_nice/get_recent_cpu/get_load_avg` (×100, nearest-int).
5. Ignore priority args & `thread_set_priority` when mlfqs.
⚠️ `ready_threads` = ready + running, excluding idle. ⚠️ Use `-mlfqs` flag (`thread_mlfqs`).
🔬 `mlfqs-load-1/60/avg`, `mlfqs-recent-1`, `mlfqs-fair-2/20`, `mlfqs-nice-2/10`, `mlfqs-block`.

## Checklist & Docs
- [ ] Phase 1  - [ ] Phase 2  - [ ] Phase 3  - [ ] Phase 4
- [ ] `pintos/src/threads/DESIGNDOC` (data structures, algorithms, synchronization, rationale)
- Run: `cd /pintos/src/threads && make check` (inside Docker).
