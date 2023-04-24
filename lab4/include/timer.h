#ifndef _TIMER_H
#define _TIMER_H

#define CORE0_TIMER_IRQ_CTRL 0x40000040

struct timer_cb {
    struct timer_cb *next;
    unsigned int expire_time;
    void (*timer_callback)(void*);
    void *arg;
};

void core_timer_enable();
void core_timer_disable();
void set_timer(unsigned int rel_time);
unsigned int read_timer();
unsigned int read_freq();
void add_timer(void (*func)(void*), void*, unsigned int);
void pop_timer();
void test_multiplex();
void ct_enable_fortt();
void two_test();

void time_elapsed();
void print_timer(void *);

#endif