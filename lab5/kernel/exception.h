#ifndef EXCEPTION_H
#define EXCEPTION_H

extern void branch_to_address_el0(void* adr, void* sp);
extern void set_exception_vector_table(void);
void print_exception_info(void);
void system_call(void);
void irq_64_el0(void);
void irq_64_el1(void);
void enable_2nd_level_interrupt_ctrl(void);
void enable_interrupts_in_el1(void);
void disable_interrupts_in_el1(void);

#endif /* EXCEPTION_H */