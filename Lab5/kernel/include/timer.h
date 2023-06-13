#ifndef TIMER_H
#define TIMER_H

#include "list.h"

#define CORE0_TIMER_IRQ_CTRL 0x40000040

typedef void (*timer_callback_t)(char *);

typedef struct timer_event
{
    list_head_t listhead;
    unsigned long long expire_time; 
    timer_callback_t callback;
    char *args;
} timer_event_t;

void init_timer_list();
void enable_core_timer();
void core_timer_handler();
void add_timer(timer_callback_t callback, char * msg, unsigned long long expire_time);
void pop_timer();
void set_core_timer_interrupt(unsigned long long time);
void set_core_timer_interrupt_to_first();
void core_timer_interrupt_enable();
void core_timer_interrupt_disable();
void core_timer_interrupt_disable_alternative();
unsigned long long get_clock_freq();
unsigned long long get_current_tick();
unsigned long long get_clock_time();
void set_cpu_timer_up();

#endif