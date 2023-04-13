#ifndef SHELL_H
#define SHELL_H

extern char *dtb_base;

void shell();
void cmd_resolve(char* cmd);
void print_system_messages();

void clear();
void print_boot_messages();
void print_system_messages();

#endif