#ifndef TIMER_H
#define TIMER_H

#include "list.h"
//https://github.com/Tekki/raspberrypi-documentation/blob/master/hardware/raspberrypi/bcm2836/QA7_rev3.4.pdf p13
#define CORE0_TIMER_IRQ_CTRL 0x40000040

typedef struct timer_event
{
    struct list_head listhead;

    unsigned long long interrupt_time; //store as tick time after cpu start

    void *callback; // interrupt -> timer_callback -> callback(args)

    char *args; // need to free the string by event callback function
} timer_event_t;

struct list_head *timer_event_list; // first head has nothing, store timer_event_t after it

void core_timer_enable();
void core_timer_disable();
void core_timer_handler();

//now the callback only support "funcion(char *)", char* in args (defualt by second)
void add_timer(void *callback, unsigned long long timeout, char *args, int bytick);
unsigned long long get_tick_plus_s(unsigned long long second);
void set_core_timer_interrupt(unsigned long long expired_time);
void set_core_timer_interrupt_by_tick(unsigned long long tick);
void two_second_alert(char *str);
void timer_list_init();
int timer_list_get_size();

#endif