#include "timer.h"
#include "uart.h"
#include "allocate.h"
static struct event* event_head = 0;

void time_elapsed(){
  unsigned int sec;
  volatile unsigned int cntpct_el0;
  asm volatile("mrs %0, cntpct_el0"
               : "=r" (cntpct_el0));
  volatile unsigned long cntfrq_el0;
  asm volatile("mrs %0, cntfrq_el0"
               : "=r" (cntfrq_el0));
  
  sec = cntpct_el0 / cntfrq_el0;
  uart_puts("Second after boot: ");
  uart_hex(sec);
  uart_puts("\n\r");
}

unsigned long get_current_time() {
  volatile unsigned long cur;
  asm volatile("mrs %0, cntpct_el0"
               : "=r" (cur));
  return cur;
}

unsigned long get_cpu_freq() {
  volatile unsigned long frq;
  asm volatile("mrs %0, cntfrq_el0"
               : "=r" (frq));
  return frq;
}

void sort_timer_event(struct event* add_event){
  struct event* pre = event_head;
  struct event* next = event_head;
  while(next != 0 && pre->expired_time < add_event->expired_time){
    pre = next;
    next = next->next;
  }
  pre->next = add_event;
  add_event->next = next;
}

void add_timer(void (*callback)(char*), char* msg, unsigned long timeout){
  struct event* add_event =(struct event * )simple_malloc(sizeof(struct event));
  unsigned long cntpct_el0 = get_current_time();
  add_event->expired_time = timeout + cntpct_el0;
  add_event->callback = callback;
  add_event->msg = msg;
  add_event->next = 0;
  if(event_head == 0){
    event_head = add_event;
    // update timer interrupt
    set_timer_expire(timeout);
  }
  else if (event_head->expired_time > add_event->expired_time){
    add_event->next = event_head;
    event_head = add_event;
    // update timer interrupt
    set_timer_expire(timeout);
  }
  else{
    // arrange the event in ascending order in terms of expired timeout
    sort_timer_event(add_event);
  }
  asm volatile("msr DAIFClr, 0xf");
}

void core_timer_handle(){
  if(event_head == 0){
    two_sec_interrupt();
  }
  else{
    struct event* select = event_head;
    event_head = event_head->next;
    select->callback(select->msg);
    volatile unsigned int cur_time;
    asm volatile("mrs %0, cntpct_el0"
                : "=r" (cur_time));
    set_timer_expire(event_head->expired_time - cur_time);
  }
}

void timer_print_callback(char* msg){
  unsigned int sec;
  volatile unsigned int cntpct_el0;
  asm volatile("mrs %0, cntpct_el0"
               : "=r" (cntpct_el0));
  volatile unsigned long cntfrq_el0;
  asm volatile("mrs %0, cntfrq_el0"
               : "=r" (cntfrq_el0));
  
  sec = cntpct_el0 / cntfrq_el0;
  uart_puts("Second after boot: ");
  uart_hex(sec);
  uart_puts("\n\r");
  uart_puts(msg);
}

void time_multiplex_test(){
  volatile unsigned long cntfrq_el0;
  asm volatile("mrs %0, cntfrq_el0"
               : "=r" (cntfrq_el0));
  char *msg1 = (char*)simple_malloc(32);
  char *msg2 = (char*)simple_malloc(32);
  char *msg3 = (char*)simple_malloc(32);
  msg1 = "MSG1\n\r";
  msg2 = "MSG2\n\r";
  msg3 = "MSG3\n\r";
  add_timer(timer_print_callback, msg1, 3*cntfrq_el0);
  add_timer(timer_print_callback, msg2, 6*cntfrq_el0);
  add_timer(timer_print_callback, msg3, 9*cntfrq_el0);
}