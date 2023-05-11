#ifndef OSCOS_XCPT_H
#define OSCOS_XCPT_H

#define XCPT_MASK_ALL() __asm__ __volatile__("msr daifset, 0xf")
#define XCPT_UNMASK_ALL() __asm__ __volatile__("msr daifclr, 0xf")

void xcpt_set_vector_table(void);

#endif
