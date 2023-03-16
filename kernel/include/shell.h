#ifndef SHELL_H
#define SHELL_H

#include "uart.h"
#include "string.h"
#include "system.h"
#include "cpio.h"
#include "malloc.h"

extern char *dtb_base;
extern char *__heap_top;

void shell();
void welcome();
void cmd_resolve(char *cmd);

#endif