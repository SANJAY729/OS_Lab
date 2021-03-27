#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "process.h"
#include "pagedir.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"

void exit(int);
tid_t exec (const char * cmd_line);
int wait(tid_t);
int write (int,const void *, unsigned);
static void syscall_handler (struct intr_frame *);
bool valid (void * vaddr);
struct file_desc * get_file(int);
void
syscall_init (void) 
{
	lock_init(&big_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  int * p = f->esp;
	if(!valid (p))
    exit(-1);
	int syscall_number = *p;
	switch (syscall_number){
		case SYS_EXIT:
      if(!valid(p + 1))
        exit(-1);
			exit(*(p + 1));
      break;

    case SYS_EXEC:
      if(!valid(p + 1) || !valid(*(p + 1)))
        exit(-1);
      f->eax = exec(*(p + 1));
      break;
    
		case SYS_WAIT:
      if(!valid(p + 1))
      	exit(-1); 
      f->eax = wait(*(p + 1));
      break;

		case SYS_WRITE:
      if (!valid(p + 5) || !valid(p + 6) || !valid (p + 7)|| !valid(*(p + 6)))
        exit(-1);
      f->eax = write(*(p + 5),*(p + 6),*(p + 7));
      break;
    
		default:
      hex_dump(p,p,64,true);
      printf("Invalid System Call Number\n");
			exit(-1);      
			break;
  }
}
void exit (int status)
{
  struct thread * parent = thread_current()->parent;
  thread_current()->exit_code = status;
  if (!list_empty(&parent->children)){
    struct child * child = get_child(thread_current()->tid,parent);
    if (child!=NULL){
      child->ret_val = status;
      child->completed = 1;
      if (thread_current()->parent->waiting_for_child_id == thread_current()->tid)
        sema_up(&thread_current()->parent->child_sema);   
    }
  }
  thread_exit();
}

tid_t exec (const char * cmd_line)
{
  lock_acquire(&big_lock);
  tid_t tid = process_execute(cmd_line);
  lock_release(&big_lock);
  return tid;
}

int wait(tid_t id)
{
  tid_t tid = process_wait(id);
  return tid;
}

int write (int fd, const void *buffer, unsigned length)
{
  if (fd == STDOUT_FILENO){
    putbuf(buffer,length);
  }
	return length;
}

bool valid(void * vaddr)
{
  //return (is_user_vaddr(vaddr) && pagedir_get_page(thread_current()->pagedir,vaddr)!=NULL);
	if (!is_user_vaddr(vaddr))
		return false;
	if (!(pagedir_get_page(thread_current()->pagedir,vaddr)!=NULL))
		return false;
	return true;
}
