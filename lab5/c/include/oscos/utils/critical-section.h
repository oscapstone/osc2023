#ifndef OSCOS_UTILS_CRITICAL_SECTION_H
#define OSCOS_UTILS_CRITICAL_SECTION_H

#include "oscos/xcpt.h"

#define CRITICAL_SECTION_ENTER(DAIF_VAL)                                       \
  do {                                                                         \
    __asm__ __volatile__("mrs %0, daif" : "=r"(DAIF_VAL));                     \
    XCPT_MASK_ALL();                                                           \
  } while (0)

#define CRITICAL_SECTION_LEAVE(DAIF_VAL)                                       \
  do {                                                                         \
    /* Prevent the compiler from reordering memory accesses after interrupt    \
       unmasking. */                                                           \
    __asm__ __volatile__("" : : : "memory");                                   \
    __asm__ __volatile__("msr daif, %0" : : "r"(DAIF_VAL));                    \
  } while (0)

#endif
