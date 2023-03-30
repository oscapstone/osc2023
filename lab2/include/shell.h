#ifndef SHELL_H
#define SHELL_H

#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001C
#define PM_WDOG 0x3F100024

void shell_start();

void cmd_handle(char *);
void cmd_help();
void cmd_hello();
void cmd_reboot();
void cmd_hwinfo();
void cmd_ls();
void cmd_cat();
void cmd_malloc();



#endif