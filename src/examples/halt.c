/* halt.c

   Simple program to test whether running a user program works.
 	
   Just invokes a system call that shuts down the OS. */

#include <syscall.h>

int
main (void)
{
  exit(0);
  exit(0);
  halt ();
  /* not reached */
}
