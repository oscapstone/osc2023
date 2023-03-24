#include "oscos/user_program.h"

#include "oscos/libc/string.h"

// Symbols defined in the linker script.
extern char _suser[], _max_euser[], _euserstack[];

bool load_user_program(const void *const start, const size_t len) {
  const size_t max_len = _max_euser - _suser;
  if (len > max_len)
    return false;

  memcpy(_suser, start, len);
  return true;
}

void run_user_program(void) {
  // We don't need to synchronize the data cache and the instruction cache since
  // we haven't enabled any of them. Nonetheless, we still have to synchronize
  // the fetched instruction stream using the `isb` instruction.
  __asm__ __volatile__("isb");

  // - Mask all interrupts.
  // - AArch64 execution state.
  // - EL0t.
  __asm__ __volatile__("msr spsr_el1, %0" : : "r"(0x3c0));

  __asm__ __volatile__("msr elr_el1, %0" : : "r"(_suser));
  __asm__ __volatile__("msr sp_el0, %0" : : "r"(_euserstack));
  __asm__ __volatile__("eret");

  __builtin_unreachable();
}
