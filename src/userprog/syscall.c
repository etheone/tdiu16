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
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "devices/input.h"

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
      thread_exit ();
      break;
    }

    case SYS_CREATE:
    {
      printf("# DEBIG_HELP : CREATE RUN\n");
      printf("Filename: %s \n", esp[1]);
      printf("Size is: %d \n", esp[2]);
      if(filesys_create((char*)esp[1], esp[2])) {
	f->eax = true;
      } else {
	f->eax = false;
      }
      break;
    }

    case SYS_OPEN:
    {
      printf("# DEBUG_HELP : OPEN IS CALLED\n");
      char *filename = esp[1];
      struct file *fileStruct;
      printf("# DEBUG_HELP : TRYING TO OPEN THE FILE\n");
      fileStruct = filesys_open(filename);
      printf("# DEBUG_HELP : GETTING CURRENT THREAD\n");
      struct thread *currentThread = thread_current();
      printf("# DEBUG_HELP : INSERTING FILE INTO FILE_MAP\n");
      if(fileStruct != NULL) {
	int fd = map_insert(&(currentThread->file_map), fileStruct);
	printf("# DEBUG_HELP : FD IS %d \n", fd);
	f->eax = fd;	
      } else {
	f->eax = -1;
      }
      break;
    }

    case SYS_REMOVE:
    {
      f->eax = filesys_remove(esp[1]);
      
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
	//printf("# DEBUG_HELP : READ IS CALLED\n");
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
	    //printf("The key pressed: %c\n", key);
	    putbuf (&key, 1);
	    buffer[i] = key;
	  }
	  f->eax = i;
	} else if (esp[1] == STDOUT_FILENO)
	{
	  /* not STDIN_FILENO, return -1 for now */
	  f->eax = -1;
	} else
	{
	  struct thread *currentThread = thread_current();
	  int fd = esp[1];
	  struct file* fileStruct = map_find(&(currentThread->file_map), fd);
	  if(fileStruct != NULL) {
	    f->eax = file_read(fileStruct, esp[2], esp[3]);
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
	    f->eax = file_write(fileStruct, esp[2], esp[3]);
	  } else {
	    f->eax = -1;
	  }
	    
	    //f->eax = -1;
	}
	break;
      }
    default:
    {
      printf ("Executed an unknown system call!\n");
      
      printf ("Stack top + 0: %d\n", esp[0]);
      printf ("Stack top + 1: %d\n", esp[1]);
      
      thread_exit ();
    }
  }
}
