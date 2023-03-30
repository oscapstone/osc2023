#ifndef SHELL_H
#define SHELL_H

#define CMD_MAX_LEN 32
void cli_cmd_clear(char*, int);
void shell_init();
void shell_input(char *cmd);
void shell_controller(char *cmd);

#endif