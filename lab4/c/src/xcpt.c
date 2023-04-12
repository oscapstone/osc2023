#include "oscos/xcpt.h"

extern char vector_table[];

void set_vector_table(void) {
  __asm__ __volatile__("msr vbar_el1, %0" : : "r"(vector_table));
}
