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
int write (int fd, const void *buffer, unsigned length);
static void syscall_handler (struct intr_frame *);
bool valid (void * vaddr);
//change4
struct file_data * get_file(int);
bool create (const char* file, unsigned initial_size);
bool remove (const char * file);
int open (const char * file);
int filesize (int fd);
int read (int fd, void * buffer, unsigned length);
void close(int fd);
//change4
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

    //change4
    case SYS_CREATE:
      if(!valid(p+4) || !valid(p+5) || !valid(*(p+4)))
        exit(-1);
      f->eax = create (*(p+4), *(p+5));
      break;
    
    case SYS_REMOVE:
      if(!valid(p+1) || !valid(*(p+1)))
        exit(-1);
      f->eax = remove(*(p+1));
      break;
    
    case SYS_OPEN:
      if(!valid (p+1) || !valid(*(p+1)))
        exit(-1);
      f->eax = open (*(p+1));
      break;
   
    case SYS_FILESIZE:
      if (!valid(p+1))
        exit(-1);
      f->eax = filesize(*(p+1));
      break;

    case SYS_READ:
      if (!valid(p+5) || !valid (p+6) || !valid (p+7) || !valid (*(p+6)))
        exit(-1);
      f->eax=read(*(p+5),*(p+6),*(p+7));
      break;
    
    case SYS_CLOSE:
      if (!valid(p+1))
        exit(-1);
      close(*(p+1));
      break;
    //change4
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
    return length;
  }
	//change4
  struct file_data * fd_elem = get_file(fd);
  if(fd_elem == NULL)
    return -1;
  lock_acquire(&big_lock);
  int temp = file_write(fd_elem->fptr,buffer,length);
  lock_release(&big_lock);
  return temp;
  //change4
}

//change4
bool create (const char * file, unsigned initial_size)
{
  if (file == NULL)
    return -1;
  lock_acquire(&big_lock);
  int ret = filesys_create(file,initial_size);
  lock_release(&big_lock);
  
  return ret;
}

bool remove (const char * file)
{
  if (file == NULL)
    return -1;
  lock_acquire(&big_lock);
  bool flag = filesys_remove(file);
  lock_release(&big_lock);

  return flag;
}

int open (const char * file)
{
  lock_acquire(&big_lock);
  struct file * fp = filesys_open (file);
  lock_release(&big_lock);
  if (fp == NULL) 
    return -1;
  struct file_data * fd_elem = malloc (sizeof(struct file_data));
  fd_elem->fd = ++thread_current()->file_count;
  fd_elem->fptr = fp;
  list_push_front(&thread_current()->files,&fd_elem->elem);
  return fd_elem->fd;
}

int filesize (int fd)
{
  struct file_data * fd_elem = get_file(fd);
  if(fd_elem == NULL)
    return -1;
  lock_acquire(&big_lock);
  int length = file_length(fd_elem->fptr); 
  lock_release(&big_lock);
  return length;
}

int read (int fd, void * buffer, unsigned length)
{
  int len = 0;
  if (fd == STDIN_FILENO){
    while (len < length){
      *((char *)buffer+len) = input_getc();
      len++;
    }
    return len;
  }
  struct file_data * fd_elem = get_file(fd);
  if (fd_elem == NULL)
    return -1;
  lock_acquire(&big_lock);
  len = file_read(fd_elem->fptr,buffer,length);
  lock_release(&big_lock);
  return len;
}

void close (int fd)
{
  if (fd == STDIN_FILENO || fd == STDOUT_FILENO)
    return;
  struct file_data * fd_elem = get_file(fd);
  if (fd_elem == NULL)
    return -1;
  lock_acquire(&big_lock);
  file_close(fd_elem->fptr);
  lock_release(&big_lock);
  list_remove(&fd_elem->elem);
  free(fd_elem);
}

//change4
bool valid(void * vaddr)
{
	if (!is_user_vaddr(vaddr))
		return false;
	if (!(pagedir_get_page(thread_current()->pagedir,vaddr)!=NULL))
		return false;
	return true;
}
//change4
struct file_data * get_file (int fd)
{
  struct thread * curr = thread_current();
  struct list_elem * e;
  for (e=list_begin(&curr->files); e != list_end (&curr->files); e = list_next(e)){
    struct file_data * fd_elem = list_entry(e, struct file_data,elem);
    if (fd_elem->fd == fd)
      return fd_elem;
  }
  return NULL;
}
//change4
