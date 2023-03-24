#include "gpio.h"
#ifndef INTERRUPT_H
#define INTERRUPT_H

#define GPU_INT_ROUT ((volatile unsigned int *)0x4000000C)
#define CORE0_TIMER_IRQ_CTRL 0x40000040
#define CORE0_IRQ_SOURCE ((volatile unsigned int *)0x40000060)
#define _IRQ_PEND_1 ((volatile unsigned int *)0x7E00B204)
//#define IRQ_PEND_1 ((_IRQ_PEND_1) - (BUS_BASE) + (MMIO_BASE))
#define IRQ_PEND_1 ((volatile unsigned int *)0x3f00B204)
#define IRQS1 (volatile unsigned int *)0x3f00b210

int core_timer_enable(void);
int core_timer_handler(void);
int mini_uart_interrupt_enable(void);

/// Interrupt enable/disable functions.
int enable_int(void);
int disable_int(void);

/// Interrupt task queue
int task_queue_run(void);
int task_queue_add(int (*fn)(void *), void *, int);
int task_queue_preempt(void);

typedef struct task_Q {
  // The handler function or call back functions.
  int (*fn)(void *);
  // Arguments of the functions.
  void *arg;
  // Priority, the 9 means the lowest priority.
  int priority;
  // Pointer to next recored.
  struct task_Q *next;
} task_q;

#endif // INTERRUPT_H
