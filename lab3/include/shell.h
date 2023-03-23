#ifndef _SHELL_H
#define _SHELL_H

void shell_input(char *cmd);
unsigned int parse_cmd(char *cmd, void *dtb);
int str_comp(char *x, char *y);
void buf_clear(char *cmd);
void exception_entry();
void exec_prog(char * addr);

#endif /*_BASH_H*/