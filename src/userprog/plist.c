#include <stddef.h>
#include <stdio.h>

#include "plist.h"

//struct list plist;

void plist_init(struct list* plist)
{
  list_init(plist);
}

pid_t process_insert(struct process_info* process_to_insert, struct list* plist)
{
  printf("\n\n#### Inserting a process in the list plist\n");
  struct plist_elem* plist_elem = (struct plist_elem*) malloc(sizeof(struct plist_elem));
  plist_elem->pinfo = *process_to_insert;

  list_push_back(plist, &plist_elem->elem);
  
  return plist_elem->pinfo.proc_id;
}

void process_remove(pid_t id, struct list* plist)
{
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

    if(!pi->alive && !pi->parent_alive)
    {
      e = list_remove(e);
      free(pi);
    }

    e = list_next(e);
  }
}

struct process_info* process_find(pid_t id, struct list* plist)
{
  struct list_elem* e;
  for(e = list_begin(plist); e != list_end(plist);)
  {
    struct process_info* pi;
    pi = (struct process_info*)list_entry(e, struct plist_elem, elem);

    if(pi->proc_id == id)
    {
      return pi;
    }

    e = list_next(e);
  }

  return NULL;
}

void plist_print(struct list* plist)
{
  struct list_elem* e;
  printf("proc_id\tparent_id\texit_status\talive\tparent_alive\tFree\n");
   for(e = list_begin(plist); e != list_end(plist);)
  {
    struct process_info* pi;
    pi = (struct process_info*)list_entry(e, struct plist_elem, elem);
    printf("%d\t%d\t%d\t%d\t%d\t%d\n", pi->proc_id, pi->parent_id, pi->exit_status, pi->alive, pi->parent_alive, pi->free);
 
    
    e = list_next(e);
  }
}


