#ifndef _IRQ_H
#define _IRQ_H

#include <type.h>
#define ENABLE_IRQS1 0x3F00B210
void default_exception_handler(uint32);
void enable_irq1();
void irq_handler();


#endif