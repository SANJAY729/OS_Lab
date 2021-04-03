#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <stdio.h>
#include "lib/kernel/list.h"

void syscall_init (void);

//change3
struct lock big_lock;
//change3
//change4
struct file_data{
	struct file* fptr;
	int fd;
	struct list_elem elem;
};
//change4

#endif /* userprog/syscall.h */
