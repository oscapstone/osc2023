#ifndef _SHELL_H_
#define _SHELL_H_

#define CMD_MAX_LEN 0x100
#define MSG_MAX_LEN 0x100

typedef struct CLI_CMDS
{
    char command[CMD_MAX_LEN];
    char help[MSG_MAX_LEN];
} CLI_CMDS;

int  cli_cmd_strcmp(const char*, const char*);
void cli_cmd_clear(char*, int);
void cli_cmd_read(char*);
void cli_cmd_exec(char*);
void shell_init(); //init uart, shell.
void shell_input(); //handler console input
void cli_cmd_clear();
void shell_command(); //handle 'command'
void str_compare();
int str_cmp();
int strcmp();
unsigned long long strlen();

#endif