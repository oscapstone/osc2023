#ifndef TIMER_H
#define TIMER_H

// the event are maintained as a queue
struct event{
  struct event* next;
  unsigned int expired_time;
  void (*callback)(void*);
  void* msg;
};

extern void core_timer_enable();
extern void two_sec_interrupt();
extern void set_timer_expire(unsigned int timeout);
void time_elapsed();
void add_timer(void (*callback)(char*), char* msg, unsigned long timeout);
void core_timer_handle();
void time_multiplex_test();
unsigned long get_cpu_freq();
unsigned long get_current_time();
void timer_print_callback(char* msg);
#endif