#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);

bool verify_variable_length(char*);

bool verify_fix_length(void*, int);

void exit_process(int);
#endif /* userprog/syscall.h */
