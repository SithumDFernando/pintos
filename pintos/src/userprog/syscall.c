#include "userprog/syscall.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>
#include "devices/input.h"
#include "devices/shutdown.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "lib/kernel/list.h"
#include "threads/interrupt.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"

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
   memory (at most MAX_USER_STRING bytes including the NUL). */
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

/* exec (cmd_line).  Runs the executable given in cmd_line and returns
   its process id (or -1 if it cannot be loaded). */
static int
sys_exec (const char *cmd_line)
{
  if (!valid_string (cmd_line))
    syscall_exit (-1);
  return process_execute (cmd_line);
}

/* wait (pid).  Waits for a direct child process to exit and returns
   its exit status. */
static int
sys_wait (tid_t pid)
{
  return process_wait (pid);
}

/* create (file, initial_size).  Creates a new file named file with
   the given initial size. */
static bool
sys_create (const char *file, unsigned initial_size)
{
  bool success;

  if (!valid_string (file))
    syscall_exit (-1);

  lock_acquire (&fs_lock);
  success = filesys_create (file, initial_size);
  lock_release (&fs_lock);

  return success;
}

/* remove (file).  Deletes the file named file. */
static bool
sys_remove (const char *file)
{
  bool success;

  if (!valid_string (file))
    syscall_exit (-1);

  lock_acquire (&fs_lock);
  success = filesys_remove (file);
  lock_release (&fs_lock);

  return success;
}

/* open (file).  Opens the file named file and returns its descriptor,
   or -1 if the file could not be opened. */
static int
sys_open (const char *file)
{
  struct file *f;
  int fd;

  if (!valid_string (file))
    syscall_exit (-1);

  lock_acquire (&fs_lock);
  f = filesys_open (file);
  lock_release (&fs_lock);

  if (f == NULL)
    return -1;

  fd = fd_alloc (f);
  return fd;
}

/* filesize (fd).  Returns the size, in bytes, of the open file fd. */
static int
sys_filesize (int fd)
{
  struct file *file = fd_lookup (fd);
  int len;

  if (file == NULL)
    return -1;

  lock_acquire (&fs_lock);
  len = file_length (file);
  lock_release (&fs_lock);

  return len;
}

/* read (fd, buffer, size).  Reads size bytes from the open file fd
   into buffer.  Returns the number of bytes actually read, 0 at EOF,
   or -1 on error. */
static int
sys_read (int fd, void *buffer, unsigned size)
{
  struct file *file;
  int bytes_read;

  if (!valid_range (buffer, size))
    syscall_exit (-1);

  if (size == 0)
    return 0;

  /* Stdin: keyboard input.  Do not hold fs_lock. */
  if (fd == 0)
    {
      uint8_t *buf = buffer;
      unsigned i;
      for (i = 0; i < size; i++)
        buf[i] = input_getc ();
      return size;
    }

  /* Stdout cannot be read. */
  if (fd == 1)
    return -1;

  file = fd_lookup (fd);
  if (file == NULL)
    return -1;

  lock_acquire (&fs_lock);
  bytes_read = file_read (file, buffer, size);
  lock_release (&fs_lock);

  return bytes_read;
}

/* write (fd, buffer, size).  Writes size bytes from buffer to the
   open file fd.  Returns the number of bytes actually written, or -1
   on error. */
static int
sys_write (int fd, const void *buffer, unsigned size)
{
  struct file *file;
  int bytes_written;

  if (!valid_range (buffer, size))
    syscall_exit (-1);

  if (size == 0)
    return 0;

  /* Stdout: console output.  Do not hold fs_lock. */
  if (fd == 1)
    {
      putbuf (buffer, size);
      return size;
    }

  /* Stdin cannot be written. */
  if (fd == 0)
    return -1;

  file = fd_lookup (fd);
  if (file == NULL)
    return -1;

  lock_acquire (&fs_lock);
  bytes_written = file_write (file, buffer, size);
  lock_release (&fs_lock);

  return bytes_written;
}

/* seek (fd, position).  Changes the next byte to be read or written
   in open file fd to position. */
static void
sys_seek (int fd, unsigned position)
{
  struct file *file = fd_lookup (fd);

  if (file != NULL)
    {
      lock_acquire (&fs_lock);
      file_seek (file, position);
      lock_release (&fs_lock);
    }
}

/* tell (fd).  Returns the position of the next byte to be read or
   written in open file fd. */
static unsigned
sys_tell (int fd)
{
  struct file *file = fd_lookup (fd);
  unsigned pos;

  if (file == NULL)
    return (unsigned) -1;

  lock_acquire (&fs_lock);
  pos = file_tell (file);
  lock_release (&fs_lock);

  return pos;
}

/* close (fd).  Closes file descriptor fd. */
static void
sys_close (int fd)
{
  if (fd < 2)
    return;
  fd_close (fd);
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
    case SYS_EXEC:
      f->eax = sys_exec ((const char *) args[0]);
      break;
    case SYS_WAIT:
      f->eax = sys_wait ((tid_t) args[0]);
      break;
    case SYS_CREATE:
      f->eax = sys_create ((const char *) args[0], (unsigned) args[1]);
      break;
    case SYS_REMOVE:
      f->eax = sys_remove ((const char *) args[0]);
      break;
    case SYS_OPEN:
      f->eax = sys_open ((const char *) args[0]);
      break;
    case SYS_FILESIZE:
      f->eax = sys_filesize ((int) args[0]);
      break;
    case SYS_READ:
      f->eax = sys_read ((int) args[0], (void *) args[1], (unsigned) args[2]);
      break;
    case SYS_WRITE:
      f->eax = sys_write ((int) args[0], (const void *) args[1], (unsigned) args[2]);
      break;
    case SYS_SEEK:
      sys_seek ((int) args[0], (unsigned) args[1]);
      break;
    case SYS_TELL:
      f->eax = sys_tell ((int) args[0]);
      break;
    case SYS_CLOSE:
      sys_close ((int) args[0]);
      break;
    default:
      /* Project 3/4 calls not implemented yet: terminate the caller. */
      syscall_exit (-1);
    }
}
