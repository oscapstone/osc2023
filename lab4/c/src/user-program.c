#include "oscos/user-program.h"

#include "oscos/console.h"
#include "oscos/libc/string.h"
#include "oscos/timer/timeout.h"
#include "oscos/utils/time.h"

// Symbols defined in the linker script.
extern char _suser[], _max_euser[], _euserstack[];

bool load_user_program(const void *const start, const size_t len) {
  const size_t max_len = _max_euser - _suser;
  if (len > max_len)
    return false;

  memcpy(_suser, start, len);
  return true;
}

static void _core_timer_el0_interrupt_handler(void *const _arg) {
  (void)_arg;

  uint64_t core_timer_freq;
  __asm__ __volatile__("mrs %0, cntfrq_el0" : "=r"(core_timer_freq));
  core_timer_freq &= 0xffffffff;

  // Print the number of seconds since boot.

  uint64_t timer_val;
  __asm__ __volatile__("mrs %0, cntpct_el0" : "=r"(timer_val));

  console_printf("# seconds since boot: %u\n",
                 (unsigned)(timer_val / core_timer_freq));

  // Set the time to interrupt to 2 seconds later.

  timeout_add_timer(_core_timer_el0_interrupt_handler, NULL, 2 * NS_PER_SEC);
}

void run_user_program(void) {
  // We don't need to synchronize the data cache and the instruction cache since
  // we haven't enabled any of them. Nonetheless, we still have to synchronize
  // the fetched instruction stream using the `isb` instruction.
  __asm__ __volatile__("isb");

  timeout_add_timer(_core_timer_el0_interrupt_handler, NULL, 2 * NS_PER_SEC);

  // - Unask all interrupts.
  // - AArch64 execution state.
  // - EL0t.
  __asm__ __volatile__("msr spsr_el1, xzr");

  __asm__ __volatile__("msr elr_el1, %0" : : "r"(_suser));
  __asm__ __volatile__("msr sp_el0, %0" : : "r"(_euserstack));
  __asm__ __volatile__("eret");

  __builtin_unreachable();
}
