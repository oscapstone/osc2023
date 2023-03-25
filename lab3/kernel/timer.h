#ifndef TIMER_H
#define TIMER_H

extern void enable_core_timer(void);
extern void reset_core_timer_in_cycle(unsigned long);
unsigned long get_current_time(void);
unsigned long get_cpu_frequency(void);
void reset_core_timer_in_second(unsigned int sec);
void print_current_time(void);

#endif /* TIMER_H */