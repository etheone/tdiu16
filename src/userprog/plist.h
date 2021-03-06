#ifndef _PLIST_H_
#define _PLIST_H_

#include <list.h>
#include <../threads/malloc.h>
#include <../threads/synch.h>


/* Place functions to handle a running process here (process list).
   
   plist.h : Your function declarations and documentation.
   plist.c : Your implementation.

   The following is strongly recommended:

   - A function that given process inforamtion (up to you to create)
     inserts this in a list of running processes and return an integer
     that can be used to find the information later on.

        - A function that given an integer (obtained from above function)
     FIND the process information in the list. Should return some
     failure code if no process matching the integer is in the list.
     Or, optionally, several functions to access any information of a
     particular process that you currently need.

   - A function that given an integer REMOVE the process information
     from the list. Should only remove the information when no process
     or thread need it anymore, but must guarantee it is always
     removed EVENTUALLY.
     
   - A function that print the entire content of the list in a nice,
     clean, readable format.
*/

#define pid_t int

/* This list is defined in '/threads/init.c'. */
extern struct list plist;
extern struct semaphore sema_plist;

struct process_info
{
  bool free;
  pid_t proc_id;
  pid_t parent_id;
  int exit_status;
  bool status_read;
  bool alive;
  bool parent_alive;
  struct semaphore sema_wait;
};

struct plist_elem
{
  struct process_info pinfo;
  struct list_elem elem;
  
 
};

void remove_child_process_after_read_exit(pid_t id, struct list* plist);

void plist_init(struct list* plist);

pid_t process_insert(struct process_info* pi, struct list* plist);

void process_remove(pid_t id, struct list* plist);

struct process_info* process_find(pid_t id, struct list* plist);

void plist_print(struct list* plist);

void plist_cleanup(struct list* plist);



#endif
