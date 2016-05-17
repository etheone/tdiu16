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

bool verify_fix_length(void* start, int length)
{
  void* end = (char*)start + length;
  void* page_check = NULL;

  while(start < end)
  {
    if(page_check == pg_round_down(start))
    {
      start = (char*) start + 1;
      continue;
    }
    
    page_check = pg_round_down(start);
    
    if(pagedir_get_page(thread_current()->pagedir, (start)) == NULL)
    {
      return false;
    }
   
    start = (char*)start + 1;
  }

  return true;
}

/* Kontrollera alla adresser från och med start till och med den
 * adress som först innehåller ett noll-tecken, `\0'. (C-strängar
 * lagras på detta sätt.) */
bool verify_variable_length(char* start)
{
  int i=0;
  unsigned pg_last = pg_no(start);
  while(true)
  {
    if(i==0 || pg_no(start+i) != pg_last)
    {
      if(pagedir_get_page(thread_current()->pagedir, (start+i)) == NULL)
	return false;
      if(*(start+i) == '\0')
        break;
    }
    else if(pg_no(start+i) == pg_last)
    {
      if(*(start+i) == '\0')
        break;
    }
    pg_last = pg_no(start+i);
    ++i;
  }
  return true;

}

void exit_process(int code)
{
  printf("# DEBUG_HELP : EXIT RUN\n");
  struct thread *currentThread = thread_current();
  map_remove_all(&(currentThread->file_map));
  struct process_info* pi = process_find(currentThread->tid, &plist);
  pi->exit_status = code;
  sema_down(&sema_plist);
  process_remove(currentThread->tid, &plist);
  sema_up(&sema_plist);
  sema_up(&pi->sema_wait);
  thread_exit ();
}

static void
syscall_handler (struct intr_frame *f) 
{
  int32_t* esp = (int32_t*)f->esp;

  if(f == NULL || esp == NULL || verify_fix_length(esp, 4) == false)
  {
    exit_process(-1);
  }

  if(esp[0] < 0 || esp[0] > SYS_NUMBER_OF_CALLS)
  {
    exit_process(-1);
  }

  int number_of_args = argc[esp[0]];

  if(verify_fix_length(esp+1, 4*number_of_args) == false) {
    exit_process(-1);
  }
  
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
      f->eax = (int)esp[1];
      exit_process((int)esp[1]);
      break;
    }

    case SYS_CREATE:
    {

      if(verify_variable_length((char*)esp[1]) == false)
      {
	exit_process(-1);
      }
      
      f->eax = filesys_create((char*)esp[1], esp[2]);
      break;
    }

    case SYS_OPEN:
    {

      if(verify_variable_length((char*)esp[1]) == false)
      {
	exit_process(-1);
      }
      
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

      if(verify_variable_length((char*)esp[1]) == false)
      {
	exit_process(-1);
      }
      
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

	if(verify_fix_length((char*)esp[2], esp[3]) == false)
	{
	  exit_process(-1);
	}
		
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

	if(verify_fix_length((char*)esp[2], esp[3]) == false)
	{
	  exit_process(-1);
	}
	
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

      if(verify_variable_length((char*)esp[1]) == false)
      {
	exit_process(-1);
      }
      
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
