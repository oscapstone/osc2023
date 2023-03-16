#ifndef SYSTEM_H
#define SYSTEM_H

#include "mailbox.h"
#include "uart.h"

#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024

int get_board_revision(unsigned int *board_revision);
int get_arm_memory_info(unsigned int *base_addr, unsigned int *size);

void reboot();
void set(long addr, unsigned int value);
void reset(int tick);
void cancel_reset();

#endif