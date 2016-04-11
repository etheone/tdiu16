#include <stddef.h>

#include "flist.h"

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
}

int map_insert(struct map* m, struct file* f)
{
  if(m != NULL) {
    int i;
    for(i = 0; i < MAP_SIZE; i++) {
      if(m->files[i] == NULL) {
	m->files[i] = f;
	return i + 2;
      }
    }
  } else {
    printf("file_map is null");
    return -1;
  }
}

struct file* map_find(struct map* m, int fd)
{
  if(fd < 2 || fd > 18)
  {
    return NULL;
  } else
  {
    return m->files[fd - 2];
  }
}

struct file* map_remove(struct map* m, int fd)
{
  if(fd < 2 || fd > 18)
  {
    return NULL;
  } else
  {
    struct file* fileToClose = m->files[fd - 2];
    m->files[fd - 2] = NULL;
    return fileToClose;
  }
}

void map_remove_all(struct map* m)
{

}  
