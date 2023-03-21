#ifndef EXCEPTION_H
#define EXCEPTION_H
extern void set_exception_vector_table(void);
int print_spsr_el1(void);
int print_elr_el1(void);
int print_esr_el1(void);
#endif // EXCEPTION_H
