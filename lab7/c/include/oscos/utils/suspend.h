#ifndef OSCOS_UTILS_SUSPEND_H
#define OSCOS_UTILS_SUSPEND_H

#include "oscos/utils/critical-section.h"

#define WFI_WHILE(COND)                                                        \
  do {                                                                         \
    bool _wfi_while_cond_val = true;                                           \
    while (_wfi_while_cond_val) {                                              \
      uint64_t _wfi_while_daif_val;                                            \
      CRITICAL_SECTION_ENTER(_wfi_while_daif_val);                             \
      if ((_wfi_while_cond_val = (COND))) {                                    \
        __asm__ __volatile__("wfi");                                           \
      }                                                                        \
      CRITICAL_SECTION_LEAVE(_wfi_while_daif_val);                             \
    }                                                                          \
  } while (0)

#endif
