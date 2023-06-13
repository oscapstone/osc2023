#include "gpio.h"

#define GPU_INT_ROUT	((volatile unsigned int*)0x4000000C)
#define CORE0_TIMER_IRQ_CTRL 0x40000040
#define CORE0_IRQ_SOURCE ((volatile unsigned int*)0x40000060)
#define _IRQ_PEND_1 ((volatile unsigned int*)0x7E00B204)
//#define IRQ_PEND_1 ((_IRQ_PEND_1) - (BUS_BASE) + (MMIO_BASE))
#define IRQ_PEND_1 ((volatile unsigned int*)0x3f00B204)
#define IRQS1 (volatile unsigned int*)0x3f00b210

int core_timer_enable(void);
int core_timer_handler(void);
int mini_uart_interrupt_enable(void);
int enable_int(void);
int disable_int(void);
