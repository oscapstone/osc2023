#ifndef __TIMER_H__
#define __TIMER_H__

struct timer_event {
    char message[256];
    unsigned int expired;
    struct timer_event *next;
};


void infinite(void);
void core_timer_enable(void);
void core_timer_disable(void);

void set_timer(unsigned int);
void print_boottime(void);


void add_core_timer(const char *message, unsigned int time);
void core_timer_handler(void);
unsigned long get_core_frequency(void);
unsigned long get_core_current_count(void);


#endif