#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <stdio.h>
#include "lib/kernel/list.h"

void syscall_init (void);

//change3
struct lock big_lock;
//change3

#endif /* userprog/syscall.h */
