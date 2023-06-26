#include "oscos/console.h"
#include "oscos/libc/inttypes.h"
#include "oscos/panic.h"

void xcpt_default_handler(const uint64_t vten) {
  uint64_t spsr_val, elr_val, esr_val;
  __asm__ __volatile__("mrs %0, spsr_el1" : "=r"(spsr_val));
  __asm__ __volatile__("mrs %0, elr_el1" : "=r"(elr_val));
  __asm__ __volatile__("mrs %0, esr_el1" : "=r"(esr_val));

  PANIC("Unhandled exception\n"
        "VTEN:     0x%8.1" PRIx64 "\n"
        "spsr_el1: 0x%.8" PRIx64 "\n"
        "elr_el1:  0x%.8" PRIx64 "\n"
        "esr_el1:  0x%.8" PRIx64,
        vten, spsr_val, elr_val, esr_val);
}
