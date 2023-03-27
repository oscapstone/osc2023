#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

void el0_irq_entry(void);
void el1h_irq_entry(void);
void exception_entry(void);
void invalid_exception_entry(void);


#endif