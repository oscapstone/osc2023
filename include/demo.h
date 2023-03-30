#include "type.h"
#ifndef __DEMO_H
#define __DEMO_H

void demo_init();
void demo_timer_interrupt();
void demo_preempt(void *ptr, uint32_t sz);
void demo_preempt_vic(void *ptr, uint32_t sz);
#endif