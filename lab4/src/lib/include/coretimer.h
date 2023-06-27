#ifndef __CORETIMER__
#define __CORETIMER__

#include <stdint.h>

#define CORE0_TIMER_IRQ_CTRL 0x40000040

typedef struct TIMER_{
    uint64_t time;
    void (*func)(void *);
    void *arg;
    struct TIMER_* next;
} TIMER;

void coretimer_el0_enable();
void coretimer_el0_handler();
void add_timer(uint64_t time_wait, void (*func)(void *), void *arg);

#endif