#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <stdio.h>
#include "lib/kernel/list.h"

struct lock big_lock;
void syscall_init (void);

struct file_desc{
  struct file * fp;
  int fd;
  struct list_elem elem;
};

#endif /* userprog/syscall.h */
