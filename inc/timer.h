#ifndef _TIMER_H
#define _TIMER_H

#include <type.h>

void timer_init();
void timer_irq_handler();
int timer_show_enable;
uint64 timer_boot_cnt;

#endif