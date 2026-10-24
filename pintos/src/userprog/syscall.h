#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <debug.h>
#include <stdbool.h>
#include "threads/synch.h"

/* Global lock serializing all file system access (the Pintos file
   system is not thread-safe).  Validate user memory BEFORE taking it
   and never hold it across process_execute/process_wait. */
extern struct lock fs_lock;

struct file;

void syscall_init (void);
void syscall_exit (int status) NO_RETURN;

/* Per-process file descriptor table (see struct thread). */
int fd_alloc (struct file *file);
struct file *fd_lookup (int fd);
bool fd_close (int fd);
void fd_close_all (void);

#endif /* userprog/syscall.h */
