#include <stddef.h>
#include <threads/synch.h>

#include "flist.h"

struct lock flist_lock;

void map_init(struct map* m)
{
  if(m != NULL) {
    int i;
    for(i = 0; i < MAP_SIZE; i++) {
      m->files[i] = NULL;
    }
  } else {
    printf("file_map is null");
  }
  //lock_init(&flist_lock);
}

int map_insert(struct map* m, struct file* f)
{
  //lock_acquire(&flist_lock);
  if(m != NULL) {
    int i;
    for(i = 0; i < MAP_SIZE; i++) {
      if(m->files[i] == NULL) {
	m->files[i] = f;
	//lock_release(&flist_lock);
	return i + 2;
      }
    }
  } else {
    printf("# file_map is null");
    //lock_release(&flist_lock);
    return -1;
  }
}

struct file* map_find(struct map* m, int fd)
{
  //lock_acquire(&flist_lock);
  if(fd < 2 || fd > 18)
  {
    //lock_release(&flist_lock);
    return NULL;
  } else
  {
    //lock_release(&flist_lock);
    return m->files[fd - 2];
  }
}

struct file* map_remove(struct map* m, int fd)
{
  //lock_acquire(&flist_lock);
  if(fd < 2 || fd > 18)
  {
    //lock_release(&flist_lock);
    return NULL;
  } else
  {
    struct file* fileToClose = m->files[fd - 2];
    m->files[fd - 2] = NULL;
    //lock_release(&flist_lock);
    return fileToClose;
  }
 
}

void map_remove_all(struct map* m)
{
  //lock_acquire(&flist_lock);
  if(m != NULL)
  {
    int i;
    for(i = 0; i < MAP_SIZE; i++) {
      if(m->files[i] != NULL) {
	file_close(m->files[i]);
      }
    }
  } else {
    printf("file_map is null");
  }
  //lock_release(&flist_lock);
  
}  
