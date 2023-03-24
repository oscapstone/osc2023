#ifndef EXCEPTION_H
#define EXCEPTION_H

extern void branch_to_address_el0(void* adr, void* sp);
extern void set_exception_vector_table(void);
void print_exception_info(void);

#endif /* EXCEPTION_H */