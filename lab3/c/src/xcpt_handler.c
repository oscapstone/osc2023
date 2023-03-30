#include <stdint.h>

#include "oscos/serial.h"

#define DUMP_SYS_REG(REGNAME, PAD)                                             \
  do {                                                                         \
    uint64_t reg_val;                                                          \
    __asm__ __volatile__("mrs %0, " #REGNAME : "=r"(reg_val));                 \
                                                                               \
    serial_fputs(#REGNAME ": " PAD "0x");                                      \
    serial_print_hex_u64(reg_val);                                             \
    serial_putc('\n');                                                         \
  } while (0)

void exception_handler(const uint64_t vten) {
  serial_fputs("VTEN:     0x");
  serial_print_hex_u64(vten);
  serial_putc('\n');

  DUMP_SYS_REG(spsr_el1, "");
  DUMP_SYS_REG(elr_el1, " ");
  DUMP_SYS_REG(esr_el1, " ");
  serial_putc('\n');
}
