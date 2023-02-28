#ifndef SHELL_H
#define SHELL_H

#define BUFFER_SIZE 100
#define BACKSPACE 8
#define DEL 127

#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001C
#define PM_WDOG 0x3F100024

void shell_start();

void cmd_init(char *, int *);
void cmd_read(char *, int *);
void cmd_handle(char *, int *);
void cmd_help();
void cmd_hello();
void cmd_reboot();

int strcmp(char *, char *);

#endif