#ifndef __SHELL_H__
#define __SHELL_H__

#include "mbox.h"
#include "uart.h"
#include "wdt.h"

#define COMMAND_SIZE 1024

void shell_input(char *);
void shell_exec(const char *);
void shell();

#endif
