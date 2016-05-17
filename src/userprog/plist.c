#include <stddef.h>
#include <stdio.h>

#include "plist.h"

//struct list plist;
struct lock plist_lock;

void plist_init(struct list* plist)
{
  list_init(plist);
  lock_init(&plist_lock);
}

pid_t process_insert(struct process_info* process_to_insert, struct list* plist)
{
  //printf("\n\n#### Inserting a process in the list plist\n");
  struct plist_elem* plist_elem = (struct plist_elem*) malloc(sizeof(struct plist_elem));
  plist_elem->pinfo = *process_to_insert;
  sema_init(&(plist_elem->pinfo.sema_wait), 0);
  lock_acquire(&plist_lock);
  list_push_back(plist, &plist_elem->elem);
  lock_release(&plist_lock);
  return plist_elem->pinfo.proc_id;
}

void process_remove(pid_t id, struct list* plist)
{
  lock_acquire(&plist_lock);
  struct list_elem* e;
  for(e = list_begin(plist); e != list_end(plist);)
  {
    struct process_info* pi;
    pi = (struct process_info*)list_entry(e, struct plist_elem, elem);

    if(pi->proc_id == id) {
      pi->alive = false;
      //e = list_remove(e);
      //free
    }
    else if(pi->parent_id == id)
    {
      pi->parent_alive = false;
      
    }
    
    e = list_next(e);
  }
  lock_release(&plist_lock);
  
}

void plist_cleanup(struct list* plist)
{
  lock_acquire(&plist_lock);
  //plist_print(plist);
  struct list_elem* e;
  for(e = list_begin(plist); e != list_end(plist);)
  {
    struct process_info* pi;
    pi = (struct process_info*)list_entry(e, struct plist_elem, elem);

    if(!pi->alive && !pi->parent_alive)
    {
      //printf("# #######process %d status read %d  parent alive %d", pi->proc_id, pi->status_read, pi->parent_alive);
      // printf("TRYING TO FREE THE WORLD");
      //printf("Here and Removing process #%d with status read %d\n", pi->proc_id, pi->status_read);
      
      e = list_remove(e);
      //printf("FREED THE WORLD");
      free(pi);
    }
    /* else if(pi->status_read == true) {
      //printf("# hej #######process %d status read %d", pi->proc_id, pi->status_read);
      //printf("Removing process #%d with status read %d\n", pi->proc_id, pi->status_read);
      e = list_remove(e);
      free(pi);
      }*/
    else {
      

      e = list_next(e);
    }
    
  }
  //plist_print(plist);
  lock_release(&plist_lock);
}

struct process_info* process_find(pid_t id, struct list* plist)
{
  lock_acquire(&plist_lock);
  struct list_elem* e;
  for(e = list_begin(plist); e != list_end(plist);)
  {
    struct process_info* pi;
    pi = (struct process_info*)list_entry(e, struct plist_elem, elem);

    if(pi->proc_id == id)
    {
      //printf("Returned process info for %d", id);
      lock_release(&plist_lock);
      return pi;
    }

    e = list_next(e);
  }
  //printf("Did not find process %d", id);
  lock_release(&plist_lock);
  return NULL;
}

void remove_child_process_after_read_exit(pid_t id, struct list* plist)
{
  lock_acquire(&plist_lock);
  struct list_elem* e;
  for(e = list_begin(plist); e != list_end(plist);)
  {
    struct process_info* pi;
    pi = (struct process_info*)list_entry(e, struct plist_elem, elem);

    if(pi->proc_id == id)
    {
      list_remove(e);
      free(pi);
    }
    e = list_next(e);
  }
  lock_release(&plist_lock);

}

void plist_print(struct list* plist)
{
  struct list_elem* e;
  printf("# proc_id\tparent_id\texit_status\talive\tparent_alive\tStatus read\tFree\n");
   for(e = list_begin(plist); e != list_end(plist);)
  {
    struct process_info* pi;
    pi = (struct process_info*)list_entry(e, struct plist_elem, elem);
    printf("# %d\t\t%d\t\t%d\t\t%d\t%d\t\t%d\t\t%d\n", pi->proc_id, pi->parent_id, pi->exit_status, pi->alive, pi->parent_alive, pi->status_read, pi->free);
    //printf("# ST: %d, PA: %d\n", pi->status_read, pi->parent_alive);
 
    
    e = list_next(e);
  }
}


