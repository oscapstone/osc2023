#ifndef __EXCEPTION__
#define __EXCEPTION__
#include <stdint.h>
#include <stddef.h>

extern uint64_t exception_vector_table;

void exception_init();
#endif