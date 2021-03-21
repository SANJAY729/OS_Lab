#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
//change3
#include <string.h>
#include "userprog/pagedir.h"
#include "filesys/directory.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/vaddr.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "lib/kernel/list.h"
#include "process.h"
//change3

static void syscall_handler (struct intr_frame *);
//change3
static struct file_data* search_fd(struct list *file_list, int fd);
static int validate_addr(const uint8_t *addr);
void proc_exit(int status);
//change3
void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  int *sys_stack = f->esp;
  validate_addr((uint8_t *) sys_stack);
  int sys_call = sys_stack[0];
  struct file_data *f_data;

  switch(sys_call){
    case SYS_EXIT:
      validate_addr((uint8_t *) (sys_stack+1));
      proc_exit(sys_stack[1]);
      break;

    case SYS_EXEC:
      validate_addr((uint8_t *) (sys_stack + 1));
      validate_addr((uint8_t *) sys_stack[1]);
      f->eax = process_execute((char *) sys_stack[1]);
      break;

    /*case SYS_WAIT:
      validate_addr((uint8_t *) (sys_stack+1));
      f->eax = process_wait(sys_stack[1]);
      break;*/

    case SYS_WRITE:
      validate_addr((uint8_t *) (sys_stack+3));
      validate_addr((uint8_t *) sys_stack[2]);
      int fd = sys_stack[1];
      char *buf = (char *) sys_stack[2];
      int size = sys_stack[3];
      if (fd == 1){
        putbuf(buf, size);
        f->eax = size;
      } 
			else {      
        f_data = search_fd(&thread_current()->open_files, fd);
        if (f_data == NULL)
          f->eax = 0;
        else if (thread_current()->curr_open_fd != -1 && f_data->fd == thread_current()->curr_open_fd){
          //printf("Trying to modilfy currently running executable");
          f->eax = 0;
        }
        else if (thread_current()->parent_open_fd != -1 && f_data->fd == thread_current()->parent_open_fd){
          //printf("Trying to modilfy parent's executable");
          f->eax = 0;
        }
        else {
          f->eax = file_write(f_data->fp, buf, size);
        }
      }
      break;
    default:
      return;
  }
}

static struct file_data * search_fd(struct list *file_list, int fd){
  struct file_data *f_data = NULL;
  struct list_elem *e;
  for (e = list_begin(file_list);e != list_end(file_list);e = list_next(e)){
    f_data = list_entry(e, struct file_data, file_elem);
    if(f_data->fd == fd)
      return f_data;
  }
  return NULL;
}

static int validate_addr(const uint8_t *addr){
  if (!is_user_vaddr(addr)){
    proc_exit(-2);
    return -1;
  }
	if (!pagedir_get_page(thread_current()->pagedir, addr)){
		proc_exit(-3);
		return -1;
	}
  return 0;
}

void proc_exit(int status){ 
  struct thread *curr = thread_current();
  /*
  while (!list_empty(&curr->child_process_list)){
    struct list_elem *e = list_pop_front(&curr->child_process_list);
    struct child *temp = list_entry(e, struct child, child_elem);
    free(temp);
  }
  */

  if (curr->parent){
    struct child *temp;
    struct list_elem *e;
    for (e = list_begin(&curr->parent->child_process_list); e != list_end(&curr->parent->child_process_list); e = list_next(e)){
      temp = list_entry(e, struct child, child_elem);
      if(temp->tid == curr->tid){
        temp->status_exit = status;
        if (temp->is_parent_wait)
          sema_up(&curr->parent->child_lock);
        break;
      }
    }
  }
  
  printf("%s: exit(%d)\n", curr->name, status);
	thread_exit();
}
