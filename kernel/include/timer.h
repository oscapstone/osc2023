#ifndef TIMER_H
#define TIMER_H

#include "list.h"
#define CORE0_TIMER_IRQ_CTRL 0x40000040

#define STR(x) #x
#define XSTR(s) STR(s)

typedef void (*timer_callback_t)(char *);

typedef struct timer_event
{
    list_head_t listhead;
    unsigned long long expire_time; 
    timer_callback_t callback; 
    char *args; 
} timer_event_t;

void timer_list_init();

void core_timer_enable();
void core_timer_disable();
void core_timer_interrupt_enable();
void core_timer_interrupt_disable();
void core_timer_interrupt_disable_alternative();

void core_timer_handler();

//now the callback only support "funcion(char *)", char* in args
void add_timer(timer_callback_t callback, char *args, unsigned long long timeout);
void pop_timer();

void two_second_alert(char *str);

/* Utility functions */

unsigned long long get_clock_tick();
unsigned long long get_clock_freq();
unsigned long long get_clock_time();
unsigned long long get_interrupt_tick();
unsigned long long get_expired_tick_relative(unsigned long long second);
void set_core_timer_interrupt(unsigned long long expired_time);
void set_core_timer_interrupt_by_tick(unsigned long long tick);

#endif