#include <stdio.h>
#include <syscall-nr.h>
#include "userprog/syscall.h"
#include "threads/interrupt.h"
#include "threads/thread.h"

/* header files you probably need, they are not used yet */
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/vaddr.h"
#include "threads/init.h"
//Added this line
#include "threads/synch.h"
///////////////
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "userprog/plist.h"
#include "devices/input.h"
#include "devices/timer.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


/* This array defined the number of arguments each syscall expects.
   For example, if you want to find out the number of arguments for
   the read system call you shall write:
   
   int sys_read_arg_count = argc[ SYS_READ ];
   
   All system calls have a name such as SYS_READ defined as an enum
   type, see `lib/syscall-nr.h'. Use them instead of numbers.
 */
const int argc[] = {
  /* basic calls */
  0, 1, 1, 1, 2, 1, 1, 1, 3, 3, 2, 1, 1, 
  /* not implemented */
  2, 1,    1, 1, 2, 1, 1,
  /* extended */
  0
};

static void
syscall_handler (struct intr_frame *f) 
{
  int32_t* esp = (int32_t*)f->esp;
  
  switch ( esp[0] )
  {
    case SYS_HALT:
    {
      printf("# DEBUG_HELP : HALT RUN\n");
      power_off ();
      printf("# DEBUG_HELP : SHOULD NEVER RUN\n");
    }
    case SYS_EXIT:
    {
      printf("# DEBUG_HELP : EXIT RUN\n");
      struct thread *currentThread = thread_current();
      map_remove_all(&(currentThread->file_map));
      //printf("1");
     
      struct process_info* pi = process_find(currentThread->tid, &plist);
      //printf("2");
      pi->exit_status = (int) esp[1];
      //printf("3");
//Fråga om detta!
      sema_down(&sema_plist);
      process_remove(currentThread->tid, &plist);
      //printf("4");
      
      //printf("5");
      sema_up(&sema_plist);
      sema_up(&pi->sema_wait);
      thread_exit ();
      break;
    }

    case SYS_CREATE:
    {
      f->eax = filesys_create((char*)esp[1], esp[2]);
      break;
    }

    case SYS_OPEN:
    {
      char *filename = (char*)esp[1];
      struct file *fileStruct = filesys_open(filename);
      struct thread *currentThread = thread_current();
      
      if(fileStruct != NULL) {
	int fd = map_insert(&(currentThread->file_map), fileStruct);

	f->eax = fd;	
      } else {
	f->eax = -1;
      }
      break;
    }

    case SYS_REMOVE:
    {
      f->eax = filesys_remove((void*)esp[1]);
      
      break;
    }

    case SYS_CLOSE:
    {
      int fd = esp[1];
      struct thread *currentThread = thread_current();
      struct file* fileToClose = map_remove(&(currentThread->file_map), fd);
      if(fileToClose != NULL)
      {
	file_close(fileToClose);
      }
      
      break;
    }
    
    case SYS_READ:
      {
	int size = esp[3];
	char* buffer = (char*)esp[2];
	if (esp[1] == STDIN_FILENO) 
	{
	  int i;
	  for(i = 0; i < size; i++)
	  {
	    char key = (char)input_getc();
	    if (key == '\r')
	    {
	      key = '\n';
	    }
	    putbuf (&key, 1);
	    buffer[i] = key;
	  }
	  f->eax = i;
	} else if (esp[1] == STDOUT_FILENO)
	{
	  f->eax = -1;
	} else
	{
	  struct thread *currentThread = thread_current();
	  int fd = esp[1];
	  struct file* fileStruct = map_find(&(currentThread->file_map), fd);
	  if(fileStruct != NULL) {
	    f->eax = file_read(fileStruct, (void*)esp[2], esp[3]);
	  } else {
	    f->eax = -1;
	  }
	}
	  
	break;
      }
    case SYS_WRITE:
      {
	if (esp[1] == STDOUT_FILENO)
	{
	    putbuf((char*)esp[2], esp[3]);
	    f->eax = esp[3];
	} else if(esp[1] == STDIN_FILENO)
	{
	  f->eax = -1;
	} else
	{
	    /* NOT STDOUT_FILENO, FILE == TRUE */
	  struct thread *currentThread = thread_current();
	  int fd = esp[1];
	  struct file* fileStruct = map_find(&(currentThread->file_map), fd);
	  if(fileStruct != NULL) {
	    f->eax = file_write(fileStruct, (void*)esp[2], esp[3]);
	  } else {
	    f->eax = -1;
	  }
	  
	}
	break;
      }

    case SYS_SEEK:
    {
      //printf("DEBUG HELP : IN SEEK \n");
      //map_find to get struct file*
      struct thread *currentThread = thread_current();
      struct file* fileStruct = map_find(&(currentThread->file_map), esp[1]);
      if(fileStruct != NULL)
      {
	file_seek(fileStruct, esp[2]);
      }
      
      break;
    }

    case SYS_TELL:
    {
      // printf("DEBUG HELP : IN TELL \n");
      //map_find to get struct file*
      struct thread *currentThread = thread_current();
      struct file* fileStruct = map_find(&(currentThread->file_map), esp[1]);
      if(fileStruct != NULL)
      {
	f->eax = file_tell(fileStruct);
      } else {
	f->eax = -1;
      }
      break;
    }

    case SYS_FILESIZE:
    {
      //printf("DEBUG HELP : IN FILESIZE \n");
      //map_find to get struct file*
      struct thread *currentThread = thread_current();
      struct file* fileStruct = map_find(&(currentThread->file_map), esp[1]);
      if(fileStruct != NULL)
      {
	f->eax = file_length(fileStruct);
      } else {
	f->eax = -1;
      }
      
      break;
    }

    case SYS_PLIST:
    {
      plist_print(&plist);
      break;
    }

    case SYS_EXEC:
    {
      f->eax = process_execute((char*)esp[1]);
      break;
    }

    case SYS_SLEEP:
    {
      timer_sleep((int64_t)esp[1]);
      break;
    }

    case SYS_WAIT:
    {
      f->eax = process_wait((int)esp[1]);
      break;
    }
    
    default:
    {
      printf ("# Executed an unknown system call!\n");
      
      printf ("# Stack top + 0: %d\n", esp[0]);
      printf ("# Stack top + 1: %d\n", esp[1]);
      
      thread_exit ();
    }
  }
}
