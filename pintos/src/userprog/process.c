#include "userprog/process.h"
#include <debug.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "userprog/gdt.h"
#include "userprog/pagedir.h"
#include "userprog/tss.h"
#include "filesys/directory.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/flags.h"
#include "threads/init.h"
#include "threads/interrupt.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/syscall.h"

static thread_func start_process NO_RETURN;
static bool load (const char *cmdline, void (**eip) (void), void **esp);
static bool extract_prog_name (const char *cmdline, char *prog, size_t size);
static void release_status (struct child_status *cs);

/* Drops one reference to CS and frees it if both parent and child
   have released it. */
static void
release_status (struct child_status *cs)
{
  int ref;

  lock_acquire (&cs->ref_lock);
  ref = --cs->ref_cnt;
  lock_release (&cs->ref_lock);

  if (ref == 0)
    free (cs);
}

/* Copies the first space-delimited token of CMDLINE into PROG
   (capacity SIZE, including the terminating null).  Returns false
   if CMDLINE has no token or the token does not fit.  Uses a
   caller-supplied buffer, so it is safe to call from any thread. */
static bool
extract_prog_name (const char *cmdline, char *prog, size_t size)
{
  size_t len = 0;

  while (*cmdline == ' ')
    cmdline++;
  while (cmdline[len] != '\0' && cmdline[len] != ' ')
    len++;
  if (len == 0 || len >= size)
    return false;
  memcpy (prog, cmdline, len);
  prog[len] = '\0';
  return true;
}

/* Starts a new thread running a user program loaded from
   FILE_NAME.  Blocks until the child process finishes loading
   its executable.  Returns the new process's thread id, or
   TID_ERROR if the thread cannot be created or if loading fails. */
tid_t
process_execute (const char *file_name) 
{
  char *fn_copy;
  char prog_name[NAME_MAX + 1];   /* Small: at most 15 bytes. */
  struct child_status *cs;
  struct thread *cur = thread_current ();
  tid_t tid;

  /* The thread is named after the program only, not the whole
     command line.  An empty command line cannot be executed. */
  if (!extract_prog_name (file_name, prog_name, sizeof prog_name))
    return TID_ERROR;

  /* Allocate child status record to coordinate with child. */
  cs = malloc (sizeof *cs);
  if (cs == NULL)
    return TID_ERROR;

  /* Make a copy of FILE_NAME (the full command line).
     Otherwise there's a race between the caller and load(). */
  fn_copy = palloc_get_page (0);
  if (fn_copy == NULL)
    {
      free (cs);
      return TID_ERROR;
    }
  strlcpy (fn_copy, file_name, PGSIZE);

  cs->tid = TID_ERROR;
  cs->exit_code = -1;
  cs->load_ok = false;
  cs->load_done = false;
  cs->fn_copy = fn_copy;
  sema_init (&cs->load_sema, 0);
  sema_init (&cs->exit_sema, 0);
  lock_init (&cs->ref_lock);
  cs->ref_cnt = 2;

  /* Create a new thread to execute the program. */
  tid = thread_create (prog_name, PRI_DEFAULT, start_process, cs);
  if (tid == TID_ERROR)
    {
      palloc_free_page (fn_copy);
      free (cs);
      return TID_ERROR;
    }

  cs->tid = tid;
  list_push_back (&cur->children, &cs->elem);

  /* Wait until child finishes loading the executable. */
  sema_down (&cs->load_sema);

  if (!cs->load_ok)
    {
      list_remove (&cs->elem);
      release_status (cs);
      return TID_ERROR;
    }

  return tid;
}

/* A thread function that loads a user process and starts it
   running. */
static void
start_process (void *aux)
{
  struct child_status *cs = aux;
  char *file_name = cs->fn_copy;
  struct intr_frame if_;
  bool success;

  thread_current ()->my_status = cs;

  /* Initialize interrupt frame and load executable. */
  memset (&if_, 0, sizeof if_);
  if_.gs = if_.fs = if_.es = if_.ds = if_.ss = SEL_UDSEG;
  if_.cs = SEL_UCSEG;
  if_.eflags = FLAG_IF | FLAG_MBS;
  success = load (file_name, &if_.eip, &if_.esp);

  /* Clean up command line page. */
  palloc_free_page (file_name);
  cs->fn_copy = NULL;

  /* Notify parent whether load succeeded. */
  cs->load_ok = success;
  cs->load_done = true;
  sema_up (&cs->load_sema);

  /* If load failed, quit. */
  if (!success) 
    thread_exit ();

  /* Start the user process by simulating a return from an
     interrupt, implemented by intr_exit (in
     threads/intr-stubs.S).  Because intr_exit takes all of its
     arguments on the stack in the form of a `struct intr_frame',
     we just point the stack pointer (%esp) to our stack frame
     and jump to it. */
  asm volatile ("movl %0, %%esp; jmp intr_exit" : : "g" (&if_) : "memory");
  NOT_REACHED ();
}

/* Waits for thread TID to die and returns its exit status.  If
   it was terminated by the kernel (i.e. killed due to an
   exception), returns -1.  If TID is invalid or if it was not a
   child of the calling process, or if process_wait() has already
   been successfully called for the given TID, returns -1
   immediately, without waiting. */
int
process_wait (tid_t child_tid) 
{
  struct thread *cur = thread_current ();
  struct list_elem *e;
  struct child_status *cs = NULL;
  int exit_code;

  for (e = list_begin (&cur->children); e != list_end (&cur->children);
       e = list_next (e))
    {
      struct child_status *c = list_entry (e, struct child_status, elem);
      if (c->tid == child_tid)
        {
          cs = c;
          break;
        }
    }

  if (cs == NULL)
    return -1;

  /* Wait for child to exit. */
  sema_down (&cs->exit_sema);

  exit_code = cs->exit_code;

  /* Remove from list so a second wait returns -1. */
  list_remove (&cs->elem);
  release_status (cs);

  return exit_code;
}

/* Free the current process's resources. */
void
process_exit (void)
{
  struct thread *cur = thread_current ();
  uint32_t *pd;

  /* A user process announces its exit status; kernel threads have
     no page directory.  Cleanup of the open files happens here too
     so that every termination path (exit or kill) is covered. */
  if (cur->pagedir != NULL)
    printf ("%s: exit(%d)\n", cur->name, cur->exit_code);
  fd_close_all ();

  /* Release child status records of all our children (orphan adoption). */
  while (!list_empty (&cur->children))
    {
      struct list_elem *e = list_pop_front (&cur->children);
      struct child_status *cs = list_entry (e, struct child_status, elem);
      release_status (cs);
    }

  /* Notify parent of our exit status and drop our reference. */
  if (cur->my_status != NULL)
    {
      struct child_status *cs = cur->my_status;
      cs->exit_code = cur->exit_code;
      if (!cs->load_done)
        {
          cs->load_ok = false;
          cs->load_done = true;
          sema_up (&cs->load_sema);
        }
      sema_up (&cs->exit_sema);
      release_status (cs);
      cur->my_status = NULL;
    }

  /* Destroy the current process's page directory and switch back
     to the kernel-only page directory. */
  pd = cur->pagedir;
  if (pd != NULL) 
    {
      /* Correct ordering here is crucial.  We must set
         cur->pagedir to NULL before switching page directories,
         so that a timer interrupt can't switch back to the
         process page directory.  We must activate the base page
         directory before destroying the process's page
         directory, or our active page directory will be one
         that's been freed (and cleared). */
      cur->pagedir = NULL;
      pagedir_activate (NULL);
      pagedir_destroy (pd);
    }
}

/* Sets up the CPU for running user code in the current
   thread.
   This function is called on every context switch. */
void
process_activate (void)
{
  struct thread *t = thread_current ();

  /* Activate thread's page tables. */
  pagedir_activate (t->pagedir);

  /* Set thread's kernel stack for use in processing
     interrupts. */
  tss_update ();
}

/* We load ELF binaries.  The following definitions are taken
   from the ELF specification, [ELF1], more-or-less verbatim.  */

/* ELF types.  See [ELF1] 1-2. */
typedef uint32_t Elf32_Word, Elf32_Addr, Elf32_Off;
typedef uint16_t Elf32_Half;

/* For use with ELF types in printf(). */
#define PE32Wx PRIx32   /* Print Elf32_Word in hexadecimal. */
#define PE32Ax PRIx32   /* Print Elf32_Addr in hexadecimal. */
#define PE32Ox PRIx32   /* Print Elf32_Off in hexadecimal. */
#define PE32Hx PRIx16   /* Print Elf32_Half in hexadecimal. */

/* Executable header.  See [ELF1] 1-4 to 1-8.
   This appears at the very beginning of an ELF binary. */
struct Elf32_Ehdr
  {
    unsigned char e_ident[16];
    Elf32_Half    e_type;
    Elf32_Half    e_machine;
    Elf32_Word    e_version;
    Elf32_Addr    e_entry;
    Elf32_Off     e_phoff;
    Elf32_Off     e_shoff;
    Elf32_Word    e_flags;
    Elf32_Half    e_ehsize;
    Elf32_Half    e_phentsize;
    Elf32_Half    e_phnum;
    Elf32_Half    e_shentsize;
    Elf32_Half    e_shnum;
    Elf32_Half    e_shstrndx;
  };

/* Program header.  See [ELF1] 2-2 to 2-4.
   There are e_phnum of these, starting at file offset e_phoff
   (see [ELF1] 1-6). */
struct Elf32_Phdr
  {
    Elf32_Word p_type;
    Elf32_Off  p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
  };

/* Values for p_type.  See [ELF1] 2-3. */
#define PT_NULL    0            /* Ignore. */
#define PT_LOAD    1            /* Loadable segment. */
#define PT_DYNAMIC 2            /* Dynamic linking info. */
#define PT_INTERP  3            /* Name of dynamic loader. */
#define PT_NOTE    4            /* Auxiliary info. */
#define PT_SHLIB   5            /* Reserved. */
#define PT_PHDR    6            /* Program header table. */
#define PT_STACK   0x6474e551   /* Stack segment. */

/* Flags for p_flags.  See [ELF3] 2-3 and 2-4. */
#define PF_X 1          /* Executable. */
#define PF_W 2          /* Writable. */
#define PF_R 4          /* Readable. */

static bool setup_stack (void **esp, const char *cmdline);
static bool validate_segment (const struct Elf32_Phdr *, struct file *);
static bool load_segment (struct file *file, off_t ofs, uint8_t *upage,
                          uint32_t read_bytes, uint32_t zero_bytes,
                          bool writable);

/* Loads an ELF executable from FILE_NAME into the current thread.
   Stores the executable's entry point into *EIP
   and its initial stack pointer into *ESP.
   Returns true if successful, false otherwise. */
bool
load (const char *cmdline, void (**eip) (void), void **esp) 
{
  struct thread *t = thread_current ();
  struct Elf32_Ehdr ehdr;
  struct file *file = NULL;
  char file_name[NAME_MAX + 1];   /* Program name only. */
  off_t file_ofs;
  bool success = false;
  int i;

  if (!extract_prog_name (cmdline, file_name, sizeof file_name))
    return false;

  /* All file system access in load() (open, read, close) is
     serialized; the lock is dropped at `done'. */
  lock_acquire (&fs_lock);

  /* Allocate and activate page directory. */
  t->pagedir = pagedir_create ();
  if (t->pagedir == NULL) 
    goto done;
  process_activate ();

  /* Open executable file. */
  file = filesys_open (file_name);
  if (file == NULL) 
    {
      printf ("load: %s: open failed\n", file_name);
      goto done; 
    }

  /* Read and verify executable header. */
  if (file_read (file, &ehdr, sizeof ehdr) != sizeof ehdr
      || memcmp (ehdr.e_ident, "\177ELF\1\1\1", 7)
      || ehdr.e_type != 2
      || ehdr.e_machine != 3
      || ehdr.e_version != 1
      || ehdr.e_phentsize != sizeof (struct Elf32_Phdr)
      || ehdr.e_phnum > 1024) 
    {
      printf ("load: %s: error loading executable\n", file_name);
      goto done; 
    }

  /* Read program headers. */
  file_ofs = ehdr.e_phoff;
  for (i = 0; i < ehdr.e_phnum; i++) 
    {
      struct Elf32_Phdr phdr;

      if (file_ofs < 0 || file_ofs > file_length (file))
        goto done;
      file_seek (file, file_ofs);

      if (file_read (file, &phdr, sizeof phdr) != sizeof phdr)
        goto done;
      file_ofs += sizeof phdr;
      switch (phdr.p_type) 
        {
        case PT_NULL:
        case PT_NOTE:
        case PT_PHDR:
        case PT_STACK:
        default:
          /* Ignore this segment. */
          break;
        case PT_DYNAMIC:
        case PT_INTERP:
        case PT_SHLIB:
          goto done;
        case PT_LOAD:
          if (validate_segment (&phdr, file)) 
            {
              bool writable = (phdr.p_flags & PF_W) != 0;
              uint32_t file_page = phdr.p_offset & ~PGMASK;
              uint32_t mem_page = phdr.p_vaddr & ~PGMASK;
              uint32_t page_offset = phdr.p_vaddr & PGMASK;
              uint32_t read_bytes, zero_bytes;
              if (phdr.p_filesz > 0)
                {
                  /* Normal segment.
                     Read initial part from disk and zero the rest. */
                  read_bytes = page_offset + phdr.p_filesz;
                  zero_bytes = (ROUND_UP (page_offset + phdr.p_memsz, PGSIZE)
                                - read_bytes);
                }
              else 
                {
                  /* Entirely zero.
                     Don't read anything from disk. */
                  read_bytes = 0;
                  zero_bytes = ROUND_UP (page_offset + phdr.p_memsz, PGSIZE);
                }
              if (!load_segment (file, file_page, (void *) mem_page,
                                 read_bytes, zero_bytes, writable))
                goto done;
            }
          else
            goto done;
          break;
        }
    }

  /* Set up stack. */
  if (!setup_stack (esp, cmdline))
    goto done;

  /* Start address. */
  *eip = (void (*) (void)) ehdr.e_entry;

  success = true;

 done:
  /* We arrive here whether the load is successful or not. */
  file_close (file);
  lock_release (&fs_lock);
  return success;
}

/* load() helpers. */

static bool install_page (void *upage, void *kpage, bool writable);

/* Checks whether PHDR describes a valid, loadable segment in
   FILE and returns true if so, false otherwise. */
static bool
validate_segment (const struct Elf32_Phdr *phdr, struct file *file) 
{
  /* p_offset and p_vaddr must have the same page offset. */
  if ((phdr->p_offset & PGMASK) != (phdr->p_vaddr & PGMASK)) 
    return false; 

  /* p_offset must point within FILE. */
  if (phdr->p_offset > (Elf32_Off) file_length (file)) 
    return false;

  /* p_memsz must be at least as big as p_filesz. */
  if (phdr->p_memsz < phdr->p_filesz) 
    return false; 

  /* The segment must not be empty. */
  if (phdr->p_memsz == 0)
    return false;
  
  /* The virtual memory region must both start and end within the
     user address space range. */
  if (!is_user_vaddr ((void *) phdr->p_vaddr))
    return false;
  if (!is_user_vaddr ((void *) (phdr->p_vaddr + phdr->p_memsz)))
    return false;

  /* The region cannot "wrap around" across the kernel virtual
     address space. */
  if (phdr->p_vaddr + phdr->p_memsz < phdr->p_vaddr)
    return false;

  /* Disallow mapping page 0.
     Not only is it a bad idea to map page 0, but if we allowed
     it then user code that passed a null pointer to system calls
     could quite likely panic the kernel by way of null pointer
     assertions in memcpy(), etc. */
  if (phdr->p_vaddr < PGSIZE)
    return false;

  /* It's okay. */
  return true;
}

/* Loads a segment starting at offset OFS in FILE at address
   UPAGE.  In total, READ_BYTES + ZERO_BYTES bytes of virtual
   memory are initialized, as follows:

        - READ_BYTES bytes at UPAGE must be read from FILE
          starting at offset OFS.

        - ZERO_BYTES bytes at UPAGE + READ_BYTES must be zeroed.

   The pages initialized by this function must be writable by the
   user process if WRITABLE is true, read-only otherwise.

   Return true if successful, false if a memory allocation error
   or disk read error occurs. */
static bool
load_segment (struct file *file, off_t ofs, uint8_t *upage,
              uint32_t read_bytes, uint32_t zero_bytes, bool writable) 
{
  ASSERT ((read_bytes + zero_bytes) % PGSIZE == 0);
  ASSERT (pg_ofs (upage) == 0);
  ASSERT (ofs % PGSIZE == 0);

  file_seek (file, ofs);
  while (read_bytes > 0 || zero_bytes > 0) 
    {
      /* Calculate how to fill this page.
         We will read PAGE_READ_BYTES bytes from FILE
         and zero the final PAGE_ZERO_BYTES bytes. */
      size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
      size_t page_zero_bytes = PGSIZE - page_read_bytes;

      /* Get a page of memory. */
      uint8_t *kpage = palloc_get_page (PAL_USER);
      if (kpage == NULL)
        return false;

      /* Load this page. */
      if (file_read (file, kpage, page_read_bytes) != (int) page_read_bytes)
        {
          palloc_free_page (kpage);
          return false; 
        }
      memset (kpage + page_read_bytes, 0, page_zero_bytes);

      /* Add the page to the process's address space. */
      if (!install_page (upage, kpage, writable)) 
        {
          palloc_free_page (kpage);
          return false; 
        }

      /* Advance. */
      read_bytes -= page_read_bytes;
      zero_bytes -= page_zero_bytes;
      upage += PGSIZE;
    }
  return true;
}

/* Writes the arguments of CMDLINE onto the (not yet installed)
   stack page KPAGE, which will be mapped at PHYS_BASE - PGSIZE,
   following the 80x86 calling convention.  On success stores the
   resulting user stack pointer in *ESP.  Fails, without touching
   anything outside KPAGE, if the arguments do not fit in one page.

   Everything is addressed as an offset OFF into KPAGE (OFF ==
   PGSIZE corresponds to PHYS_BASE), so we never depend on which
   page directory is active. */
static bool
push_arguments (uint8_t *kpage, const char *cmdline, void **esp)
{
  /* Scratch pages keep the kernel stack small. */
  char *copy = palloc_get_page (0);
  char **tok = palloc_get_page (0);   /* Tokens in COPY. */
  uint32_t *addr = palloc_get_page (0); /* User address of each string. */
  const size_t max_args = PGSIZE / sizeof (char *);
  bool success = false;

  if (copy != NULL && tok != NULL && addr != NULL)
    {
      char *token, *save_ptr;
      size_t argc = 0, str_bytes = 0, off, i;
      uint8_t *base = (uint8_t *) PHYS_BASE - PGSIZE;

      /* Pass 1: tokenize a private copy (strtok_r mutates it);
         consecutive spaces collapse into one delimiter. */
      strlcpy (copy, cmdline, PGSIZE);
      for (token = strtok_r (copy, " ", &save_ptr); token != NULL;
           token = strtok_r (NULL, " ", &save_ptr))
        {
          if (argc >= max_args)
            goto done;
          tok[argc++] = token;
          str_bytes += strlen (token) + 1;
        }

      /* Reject before writing: strings + up to 3 padding bytes +
         argv[] with its null sentinel + argv, argc, return address. */
      if (argc == 0
          || str_bytes + 3 + sizeof (char *) * (argc + 1)
             + 3 * sizeof (uint32_t) > PGSIZE)
        goto done;

      /* Pass 2: copy the strings, highest address first. */
      off = PGSIZE;
      for (i = 0; i < argc; i++)
        {
          size_t len = strlen (tok[i]) + 1;
          off -= len;
          memcpy (kpage + off, tok[i], len);
          addr[i] = (uint32_t) (base + off);
        }

      /* Word-align (page is zeroed, so padding bytes are 0). */
      off &= ~3u;

      /* argv[argc] = NULL, then argv[argc-1] ... argv[0]. */
      off -= sizeof (uint32_t);
      *(uint32_t *) (kpage + off) = 0;
      for (i = argc; i > 0; i--)
        {
          off -= sizeof (uint32_t);
          *(uint32_t *) (kpage + off) = addr[i - 1];
        }

      /* argv (address of argv[0]), argc, fake return address. */
      off -= sizeof (uint32_t);
      *(uint32_t *) (kpage + off) = (uint32_t) (base + off + sizeof (uint32_t));
      off -= sizeof (uint32_t);
      *(uint32_t *) (kpage + off) = argc;
      off -= sizeof (uint32_t);
      *(uint32_t *) (kpage + off) = 0;

      *esp = base + off;
      success = true;
    }

 done:
  palloc_free_page (copy);
  palloc_free_page (tok);
  palloc_free_page (addr);
  return success;
}

/* Create the initial stack by mapping a zeroed page at the top of
   user virtual memory and pushing the arguments of CMDLINE. */
static bool
setup_stack (void **esp, const char *cmdline) 
{
  uint8_t *kpage;
  bool success = false;

  kpage = palloc_get_page (PAL_USER | PAL_ZERO);
  if (kpage != NULL) 
    {
      /* Fill the page first so that failure only needs to free it. */
      success = push_arguments (kpage, cmdline, esp)
                && install_page (((uint8_t *) PHYS_BASE) - PGSIZE, kpage, true);
      if (!success)
        palloc_free_page (kpage);
    }
  return success;
}

/* Adds a mapping from user virtual address UPAGE to kernel
   virtual address KPAGE to the page table.
   If WRITABLE is true, the user process may modify the page;
   otherwise, it is read-only.
   UPAGE must not already be mapped.
   KPAGE should probably be a page obtained from the user pool
   with palloc_get_page().
   Returns true on success, false if UPAGE is already mapped or
   if memory allocation fails. */
static bool
install_page (void *upage, void *kpage, bool writable)
{
  struct thread *t = thread_current ();

  /* Verify that there's not already a page at that virtual
     address, then map our page there. */
  return (pagedir_get_page (t->pagedir, upage) == NULL
          && pagedir_set_page (t->pagedir, upage, kpage, writable));
}
