#ifndef TIMER_H
#define TIMER_H

#include "list.h"

#define CORE0_TIMER_IRQ_CTRL 0x40000040

typedef void (*timer_callback_t)(unsigned long long);

typedef struct timer_event
{
    list_head_t listhead;
    unsigned long long expire_time; 
    timer_callback_t callback;
} timer_event_t;

void enable_core_timer();
void core_timer_handler();
void add_timer(timer_callback_t callback, unsigned long long expire_time);
void pop_timer();
void set_core_timer_interrupt(unsigned long long time);
void set_core_timer_interrupt_to_first();
void core_timer_interrupt_enable();
void core_timer_interrupt_disable();
void core_timer_interrupt_disable_alternative();

#endif