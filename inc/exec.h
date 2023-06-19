#ifndef _EXEC_H
#define _EXEC_H

void user_prog_start(void);

// pass the user stack pointer and process memory location (program loaded)
void sched_new_user_prog(char *file_name);

void exit_user_prog(void);

void exec_user_prog(void *entry, char *user_sp, char *kernel_sp);

#endif