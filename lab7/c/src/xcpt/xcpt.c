#include "oscos/xcpt.h"

extern char xcpt_vector_table[];

void xcpt_set_vector_table(void) {
  __asm__ __volatile__("msr vbar_el1, %0" : : "r"(xcpt_vector_table));
}
