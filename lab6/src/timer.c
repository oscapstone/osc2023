#include "timer.h"
#include "uart.h"

#define TIMER_LIMIT 10

// The global timer queue
timer_Q queue[TIMER_LIMIT];

/**********************************************************
 * Add an timer event into timer Queue.
 *********************************************************/
int add_timer(int (*fn)(void *), int second, void *arg) {
  for (int i = 0; i < TIMER_LIMIT; i++) {
    if (!queue[i].used) {
      queue[i].used = 1;
      queue[i].fn = fn;
      queue[i].remain = second;
      queue[i].arg = arg;
      return 0;
    }
  }
  return 1;
}

/***********************************************************
 * The Warp of the uart_puts(), please use this function
 * in this compile unit.
 ***********************************************************/
static int warp_uart_puts(void *s) {
  uart_puts((char *)s);
  uart_puts("\n");
  return 0;
}

/***********************************************************
 * The Warp of the uart_a_gets(), please use this function
 * in this compile unit.
 ***********************************************************/
static int warp_uart_a_gets(void *s) {
  char tmp[100] = {0};
  uart_a_gets(tmp, 100);
  uart_puts("Async get: ");
  uart_puts(tmp);
  uart_puts("\n");
  return 0;
}

/***********************************************************
 * Set timeout command.
 * When timeout, this function will print the message
 **********************************************************/
int set_timeout(char *message, int second) {
  add_timer(warp_uart_puts, second, (void *)message);
  return 0;
}

/************************************************************
 * A async_read event by timer
 ***********************************************************/
int set_timer_read(char *message) {
  add_timer(warp_uart_a_gets, 8, 0);
  return 0;
}

/*************************************************************
 * Initial timer queue
 *************************************************************/
int init_timer_Q(void) {
  for (int i = 0; i < TIMER_LIMIT; i++) {
    queue[i].used = 0;
  }
  return 0;
}

/*****************************************************************
 * Walk through timer event queue and reduce the time count.
 * If timeout, run the callback function.
 ****************************************************************/
int timer_walk(int sec) {
  for (int i = 0; i < TIMER_LIMIT; i++) {
    if (!queue[i].used)
      continue;
    queue[i].remain -= sec;
    if (queue[i].remain <= 0) {
      queue[i].fn(queue[i].arg);
      queue[i].used = 0;
    }
  }
  return 0;
}

/********************************************************************
 * Disable the interrupt
 *******************************************************************/
int disable_timer_int(void) {
  asm volatile("ldr	x1, 	=0xffff000040000040;"
               "mov	x0,	0;"
               "str	w0, 	[x1];");
  return 0;
}

/*********************************************************************
 * Enable timer interrupt and temporary set the large timeout time
 * to avoid a quick interrupt again.
 *
 * The correct timer interrupt time will be set by the timer handler
 ********************************************************************/
int enable_timer_int(void) {
  asm volatile("mrs	x0, 	cntfrq_el0;"
               "mov	x1, 	100;"
               "mul	x0, 	x0, x1;"
               "msr	cntp_tval_el0, x0;"
               "mov	x0, 	2;"
               "ldr	x1, 	=0xffff000040000040;"
               "str	w0,	[x1];");
  return 0;
}

/***********************************************************************
 * Delay cycles.
 **********************************************************************/
int delay(uint64_t cycles) {
  if (cycles)
    while (cycles-- > 0) {
      asm volatile("nop;");
    }
  return 0;
}
