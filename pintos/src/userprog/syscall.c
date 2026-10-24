#include "userprog/syscall.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>
#include "devices/shutdown.h"
#include "filesys/file.h"
#include "lib/kernel/list.h"
#include "threads/interrupt.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"

/* Global file system lock; see userprog/syscall.h. */
struct lock fs_lock;

/* One open file of a process. */
struct fd_entry
  {
    int fd;                     /* Descriptor number (>= 2). */
    struct file *file;          /* Underlying open file. */
    struct list_elem elem;      /* Element in thread->fd_list. */
  };

static void syscall_handler (struct intr_frame *);

/* Number of 32-bit arguments each system call takes, indexed by
   syscall number.  Numbers outside the table (project 3/4 calls)
   are rejected. */
static const int syscall_nargs[] =
  {
    [SYS_HALT] = 0, [SYS_EXIT] = 1, [SYS_EXEC] = 1, [SYS_WAIT] = 1,
    [SYS_CREATE] = 2, [SYS_REMOVE] = 1, [SYS_OPEN] = 1,
    [SYS_FILESIZE] = 1, [SYS_READ] = 3, [SYS_WRITE] = 3,
    [SYS_SEEK] = 2, [SYS_TELL] = 1, [SYS_CLOSE] = 1,
  };
#define SYSCALL_COUNT (sizeof syscall_nargs / sizeof syscall_nargs[0])

void
syscall_init (void) 
{
  lock_init (&fs_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/* ---- User memory validation ----
   Approach: verify-then-dereference.  Every user pointer is checked
   (non-NULL, below PHYS_BASE, page mapped) before the kernel touches
   it, and always before fs_lock is taken, so a bad pointer never
   leaves a lock held.  Callers terminate the process with exit(-1)
   when a check fails. */

/* Longest user string we are willing to scan (one page). */
#define MAX_USER_STRING PGSIZE

/* True iff the single byte at UADDR is a mapped user address. */
static bool
valid_byte (const void *uaddr)
{
  return uaddr != NULL && is_user_vaddr (uaddr)
         && pagedir_get_page (thread_current ()->pagedir, uaddr) != NULL;
}

/* True iff all SIZE bytes starting at UADDR are valid user memory.
   Checks both ends (with wrap-around guard) and one byte on every
   page in between.  A zero-length range is accepted. */
static bool
valid_range (const void *uaddr, size_t size)
{
  const uint8_t *start = uaddr;
  const uint8_t *last;
  const uint8_t *page;

  if (size == 0)
    return true;
  last = start + size - 1;
  if (last < start || !valid_byte (start) || !valid_byte (last))
    return false;
  for (page = pg_round_down (start) + PGSIZE; page < last; page += PGSIZE)
    if (!valid_byte (page))
      return false;
  return true;
}

/* True iff S is a NUL-terminated string lying entirely in valid user
   memory (at most MAX_USER_STRING bytes including the NUL).  Used by
   the file and exec system calls in later phases. */
static bool valid_string (const char *s) UNUSED;
static bool
valid_string (const char *s)
{
  size_t i;

  for (i = 0; i < MAX_USER_STRING; i++)
    {
      /* Re-check only at the start and on each new page. */
      if ((i == 0 || pg_ofs (s + i) == 0) && !valid_byte (s + i))
        return false;
      if (s[i] == '\0')
        return true;
    }
  return false;
}

/* Terminates the current process with STATUS.  The "name: exit(N)"
   message and cleanup are done in process_exit(). */
void
syscall_exit (int status)
{
  thread_current ()->exit_code = status;
  thread_exit ();
}

/* ---- File descriptor table ---- */

/* Registers FILE in the current process's table and returns its new
   descriptor, or -1 (closing FILE) if memory is exhausted. */
int
fd_alloc (struct file *file)
{
  struct thread *t = thread_current ();
  struct fd_entry *e = malloc (sizeof *e);

  if (e == NULL)
    {
      lock_acquire (&fs_lock);
      file_close (file);
      lock_release (&fs_lock);
      return -1;
    }
  e->fd = t->next_fd++;
  e->file = file;
  list_push_back (&t->fd_list, &e->elem);
  return e->fd;
}

/* Returns the open file for FD in the current process, or NULL if FD
   is the console, unknown or already closed. */
struct file *
fd_lookup (int fd)
{
  struct thread *t = thread_current ();
  struct list_elem *e;

  if (fd < 2)
    return NULL;
  for (e = list_begin (&t->fd_list); e != list_end (&t->fd_list);
       e = list_next (e))
    {
      struct fd_entry *fe = list_entry (e, struct fd_entry, elem);
      if (fe->fd == fd)
        return fe->file;
    }
  return NULL;
}

/* Closes FD and frees its entry.  Returns false if FD is not open. */
bool
fd_close (int fd)
{
  struct thread *t = thread_current ();
  struct list_elem *e;

  if (fd < 2)
    return false;
  for (e = list_begin (&t->fd_list); e != list_end (&t->fd_list);
       e = list_next (e))
    {
      struct fd_entry *fe = list_entry (e, struct fd_entry, elem);
      if (fe->fd == fd)
        {
          list_remove (e);
          lock_acquire (&fs_lock);
          file_close (fe->file);
          lock_release (&fs_lock);
          free (fe);
          return true;
        }
    }
  return false;
}

/* Closes every open file of the current process (process exit). */
void
fd_close_all (void)
{
  struct thread *t = thread_current ();

  while (!list_empty (&t->fd_list))
    {
      struct fd_entry *fe = list_entry (list_pop_front (&t->fd_list),
                                        struct fd_entry, elem);
      lock_acquire (&fs_lock);
      file_close (fe->file);
      lock_release (&fs_lock);
      free (fe);
    }
}

/* ---- System calls ---- */

/* write (fd, buffer, size).  Only the console (fd 1) for now; file
   descriptors are added in the file system phase. */
static int
sys_write (int fd, const void *buffer, unsigned size)
{
  if (!valid_range (buffer, size))
    syscall_exit (-1);
  if (fd == 1)
    {
      putbuf (buffer, size);
      return size;
    }
  return -1;
}

static void
syscall_handler (struct intr_frame *f) 
{
  uint32_t args[4];
  uint32_t number;
  int nargs;

  /* Read the syscall number, then exactly the arguments it needs,
     validating the user stack first. */
  if (!valid_range (f->esp, sizeof (uint32_t)))
    syscall_exit (-1);
  number = *(uint32_t *) f->esp;
  if (number >= SYSCALL_COUNT)
    syscall_exit (-1);
  nargs = syscall_nargs[number];
  if (!valid_range (f->esp, sizeof (uint32_t) * (1 + nargs)))
    syscall_exit (-1);
  memcpy (args, (uint32_t *) f->esp + 1, sizeof (uint32_t) * nargs);

  switch (number)
    {
    case SYS_HALT:
      shutdown_power_off ();
      NOT_REACHED ();
    case SYS_EXIT:
      syscall_exit ((int) args[0]);
    case SYS_WRITE:
      f->eax = sys_write ((int) args[0], (const void *) args[1], args[2]);
      break;
    default:
      /* Not implemented yet (later phases): terminate the caller. */
      syscall_exit (-1);
    }
}
