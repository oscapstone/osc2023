#ifndef __TIMER_H__
#define __TIMER_H__

struct timer_event {
    void *args;
    unsigned int expired;
    void (*callback)(void*);
    struct timer_event *next;
};

void infinite(void);
void core_timer_enable(void);
void core_timer_disable(void);
void add_core_timer(void (*func)(void*), void *args, unsigned int time);
void set_core_timer(unsigned int time);
void remove_core_timer(void);
void print_elapsed_time(void);
void print_core_timer_message(void *args);
unsigned long get_core_frequency(void);
unsigned long get_core_current_count(void);

#endif