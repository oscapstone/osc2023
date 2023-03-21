#ifndef INTERRUPT_H
#define INTERRUPT_H

#define CORE0_TIMER_IRQ_CTRL 0x40000040
int core_timer_enable(void);
int core_timer_handler(void);
#endif	// INTERRUPT_H
