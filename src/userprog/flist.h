#ifndef _MAP_H_
#define _MAP_H_

/* Place functions to handle a process open files here (file list).
   
   flist.h : Your function declarations and documentation.
   flist.c : Your implementation.

   The following is strongly recommended:

   - A function that given a file (struct file*, see filesys/file.h)
     and a process id INSERT this in a list of files. Return an
     integer that can be used to find the opened file later.

   - A function that given an integer (obtained from above function)
     and a process id FIND the file in a list. Should return NULL if
     the specified process did not insert the file or already removed
     it.

   - A function that given an integer (obtained from above function)
     and a process id REMOVE the file from a list. Should return NULL
     if the specified process did not insert the file or already
     removed it.
   
   - A function that given a process id REMOVE ALL files the specified
     process have in the list.

   All files obtained from filesys/filesys.c:filesys_open() are
   considered OPEN files and must be added to a list or else kept
   track of, to guarantee ALL open files are eventyally CLOSED
   (probably when removed from the list(s)).
 */

#define MAP_SIZE 16
struct map
{
  struct file* files[MAP_SIZE];
};

/* c-constructor for map */
void map_init(struct map* m);

/* insert file* f into map m, returns a filedescriptor */
int map_insert(struct map* m, struct file* f);

/* find file* f given a fd, NULL is returned if not found */
struct file* map_find(struct map* m, int fd);

/* removes and returns file* f for file descriptor fd, NULL returned if NOT found */
struct file* map_remove(struct map* m, int fd);

/* removes all files in map m */
void map_remove_all(struct map* m);




#endif
