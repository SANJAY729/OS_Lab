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
void kill (void);
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
    kill();
	int syscall_number = *p;
	switch (syscall_number){
		case SYS_EXIT:
      if(!valid(p+1))
        kill();
			exit(*(p+1));
      break;

    case SYS_EXEC:
      if(!valid(p+1) || !valid(*(p+1)))
        kill();
      f->eax = exec(*(p+1));
      break;
    
		case SYS_WAIT:
      if(!valid(p+1))
        kill(); 
      f->eax = wait(*(p+1));
      break;

		case SYS_WRITE:
      if (!valid(p+5) || !valid(p+6) || 
            !valid (p+7)|| !valid(*(p+6)))
        kill();
      f->eax = write(*(p+5),*(p+6),*(p+7));
      break;
    
		default:
      /* Invalid System Call Number | we kill the process */
      hex_dump(p,p,64,true);
      printf("Invalid System Call number\n");
			kill();      
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
      child->ret_val=status;
      child->used = 1;
      if (thread_current()->parent->waiton_child == thread_current()->tid)
        sema_up(&thread_current()->parent->child_sem);   
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
    return length;
  }
  struct file_desc * fd_elem = get_file(fd);
  if(fd_elem == NULL)
    return -1;
  lock_acquire(&big_lock);
  int ret = file_write(fd_elem->fp,buffer,length);
  lock_release(&big_lock);
  return ret;
}

bool valid(void * vaddr)
{
  return (is_user_vaddr(vaddr) && pagedir_get_page(thread_current()->pagedir,vaddr)!=NULL);
}

void kill () 
{
  exit(-1); 
}

/* Simple function to traverse current threads
  file list and return file_desc element's pointer
  equivalent to fd */
struct file_desc * get_file (int fd)
{
  struct thread * curr = thread_current();
  struct list_elem * e;
  for (e=list_begin(&curr->file_list); e != list_end (&curr->file_list); e = list_next(e)){
    struct file_desc * fd_elem = list_entry(e, struct file_desc,elem);
    if (fd_elem->fd == fd)
      return fd_elem;
  }
  return NULL;
}
