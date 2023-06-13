#include <stdint.h>

#include "oscos/console.h"
#include "oscos/libc/inttypes.h"

#define DUMP_SYS_REG(REGNAME, PAD)                                             \
  do {                                                                         \
    uint64_t reg_val;                                                          \
    __asm__ __volatile__("mrs %0, " #REGNAME : "=r"(reg_val));                 \
                                                                               \
    console_printf(#REGNAME ": " PAD "0x%.8" PRIx64 "\n", reg_val);            \
  } while (0)

void xcpt_default_handler(const uint64_t vten) {
  console_printf("VTEN:     0x%8.1" PRIx64 "\n", vten);

  DUMP_SYS_REG(spsr_el1, "");
  DUMP_SYS_REG(elr_el1, " ");
  DUMP_SYS_REG(esr_el1, " ");
  console_putc('\n');
}
