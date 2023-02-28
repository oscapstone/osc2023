#ifndef _SHELL_H
#define _SHELL_H

void shell_input(char *cmd);
unsigned int parse_cmd(char *cmd);
int str_comp(char *x, char *y);
void buf_clear(char *cmd);

#endif /*_BASH_H*/