# Lab 0 — Getting Real: Kernel Monitor & Boot Tracing

**Spec**: [P0_kernal_monitor.md](../labs/P0_kernal_monitor.md) · **Weight**: 30% code / 70% documentation
**Files**: `pintos/src/threads/init.c` (code) · `pintos/src/p0/DESIGNDOC` + screenshots (docs)

## 1. Goal

1. Boot Pintos under QEMU (and Bochs if available) and capture evidence.
2. Trace the first instructions with GDB (BIOS → `loader.S` → `pintos_init()`).
3. Add an interactive kernel monitor (`CS318> ` prompt) used when no kernel
   command line is passed.

## 2. Phase A — Environment & Boot

| Step | Command (inside container) | Evidence |
|------|----------------------------|----------|
| Build image | `docker build -t pintos .` (host) | — |
| Run w/ mount | `docker run -it --rm -v "${PWD}/pintos/src:/pintos/src" pintos` | — |
| Build kernel | `cd /pintos/src/threads && make` | no errors |
| Boot QEMU | `cd build && pintos -- run alarm-zero` | screenshot with `Boot complete.` |
| Boot Bochs | `pintos --bochs -- run alarm-zero` | only if Bochs image built (`Dockerfile.bak` / removed `Dockerfile_Bochs`) |

⚠️ Bochs was deliberately removed from the current `Dockerfile` (README
step 2). Decide whether the module requires Bochs screenshots; if so the
image must be rebuilt using the Bochs variant (`Dockerfile_Bochs` is deleted
in the working tree — recover with `git show HEAD:Dockerfile_Bochs`).

⚠️ Docker Desktop must be running (`docker ps` failed earlier).

## 3. Phase B — GDB Tracing (documentation heavy)

Two terminals attached to the same container (`docker exec -it <id> bash`).

```
T1: pintos --gdb -- run alarm-zero
T2: pintos-gdb build/kernel.o ; (gdb) target remote localhost:1234
```

### B1. BIOS
- `(gdb) x/5i $pc` right after attach. Record: first instruction
  (`ljmp $0xf000,$0xe05b` style), its physical address (`0xFFFFFFF0`), why
  (reset vector at top of 4 GB, BIOS ROM aliasing), next 3 instructions.

### B2. Bootloader
- `b *0x7c00`, `c`; step through `pintos/src/threads/loader.S`; compare with
  `pintos/src/threads/build/loader.asm` using `x/i`.
- Answer: INT 13h (`ah=0x42` extended read) disk read; MBR partition scan
  looks for Pintos kernel partition type `0x20`; failure path prints
  `Not found` / reboot (`int $0x18`); handoff by `ljmp`/`call *%eax` into
  kernel entry `start` at `0x20000+` after loading.
  ➜ Verify each claim in the real `pintos/src/threads/loader.S` before writing.

### B3. Kernel
- `b pintos_init`; read `pintos/src/threads/init.c`.
- Inspect `init_page_dir[pd_no(ptov(0))]` (`p/x`), then at the 1st and 3rd
  call of `palloc_get_page` (`b palloc_get_page`, use `ignore` counts):
  `bt`, `finish` → return value, re-print page dir entry.
- Keep a transcript of every GDB command and output (paste into DESIGNDOC).

## 4. Phase C — Kernel Monitor (`pintos/src/threads/init.c`, TODO at line ~136)

```c
} else {
  /* No command line: run an interactive monitor. */
  monitor ();
}
```

Add a `static void monitor (void)`:

```
loop:
  printf ("CS318> ");
  read line: c = input_getc();
    '\r' or '\n'  -> echo '\n', terminate buffer
    backspace(0x7f/'\b') -> drop last char, echo "\b \b" (optional nicety)
    else          -> append if space left, echo char (input_getc does NOT echo)
  if line == "whoami" -> printf ("<your name>\n")
  elif line == "exit" -> break
  elif empty          -> continue (no error)   // decide & document
  else                -> printf ("invalid command\n")
```

Design points to justify in DESIGNDOC:
- Fixed-size buffer (e.g. 80) on kernel stack is fine (< 4 KB limit) — reject
  overflow rather than corrupt memory.
- Use `input_getc()` (blocks on keyboard queue) + `printf`/`putchar`.
- `exit` breaks the loop and falls through to the existing `shutdown()`.
- Compare strings with `strcmp` (`pintos/src/lib/string.c`).

✅ `pintos -- ` (no actions) → prompt; `whoami` prints name; `foo` → `invalid command`; `exit` powers off.
⚠️ Under QEMU with `-serial stdio` input arrives via serial; ensure `\r` is handled.
🔬 Manual test via `pintos --` inside Docker; add a screenshot.

## 5. Phase D — Documentation (`pintos/src/p0/DESIGNDOC`)

Sections: GDB answers (B1–B3) with command transcripts; monitor design
rationale; screenshots in `pintos/src/p0/`.

## 6. Checklist

- [ ] QEMU boot screenshot  - [ ] Bochs boot screenshot (if required)
- [ ] B1 answers  - [ ] B2 answers  - [ ] B3 answers
- [ ] monitor implemented  - [ ] DESIGNDOC written
