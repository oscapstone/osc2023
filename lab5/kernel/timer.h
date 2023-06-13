#ifndef TIMER_H
#define TIMER_H

extern void enable_core_timer(void);
unsigned long get_current_time(void);
unsigned long get_cpu_frequency(void);

extern void reset_core_timer_in_cycle(unsigned long);
extern void reset_core_timer_absolute(unsigned long);
void reset_core_timer_in_second(unsigned int sec);

void print_current_time(void);
void timer_expired(void);

void add_timer(void (*callback)(void*), void *arg, int sec);
void cmd_add_timer(char* cmd);

#endif /* TIMER_H */