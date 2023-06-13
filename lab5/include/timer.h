#ifndef _TIMER_H
#define _TIMER_H

#define CORE0_TIMER_IRQ_CTRL 0x40000040

void timer_init();
void handle_timer_irq();
void core_timer_enable();
void core_timer_disable();
void set_timer(unsigned int rel_time);
unsigned int read_timer();
unsigned int read_freq();

#endif