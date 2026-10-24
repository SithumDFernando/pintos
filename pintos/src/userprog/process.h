#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include "threads/synch.h"

/* Record shared between a parent process and its child to track
   exit status, load status, and synchronize termination. */
struct child_status
  {
    tid_t tid;                  /* Child's thread ID. */
    int exit_code;              /* Status given to exit(); -1 by default. */
    bool load_ok;               /* Set by child when load() completes. */
    bool load_done;             /* True once load outcome is published. */
    char *fn_copy;              /* Command line copy passed to child. */
    struct semaphore load_sema; /* Parent blocks until child finishes load. */
    struct semaphore exit_sema; /* Parent blocks until child exits. */
    int ref_cnt;                /* Reference count: 2 (parent + child). */
    struct lock ref_lock;       /* Protects ref_cnt. */
    struct list_elem elem;      /* Element in parent's children list. */
  };

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

#endif /* userprog/process.h */
