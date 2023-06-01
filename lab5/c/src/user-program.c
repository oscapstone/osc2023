#include "oscos/user-program.h"

#include "oscos/console.h"
#include "oscos/libc/string.h"
#include "oscos/mem/page-alloc.h"
#include "oscos/mem/vm.h"
#include "oscos/timer/timeout.h"
#include "oscos/utils/math.h"
#include "oscos/utils/time.h"

#define USER_STACK_ORDER 23 // 8MB.
#define USER_STACK_BLOCK_ORDER (USER_STACK_ORDER - PAGE_ORDER)

static void *_user_program_start, *_user_stack_end;

bool load_user_program(const void *const start, const size_t len) {
  // Allocate page frames for the user program.

  const size_t user_program_n_pages =
      (len + ((1 << PAGE_ORDER) - 1)) >> PAGE_ORDER;
  const size_t user_program_block_order = clog2(user_program_n_pages);
  const spage_id_t user_program_page_id = alloc_pages(user_program_block_order);
  if (user_program_page_id < 0) { // Out of memory.
    return false;
  }
  _user_program_start = pa_to_kernel_va(page_id_to_pa(user_program_page_id));

  // Allocate page frames for the user stack.

  const spage_id_t user_stack_page_id = alloc_pages(USER_STACK_BLOCK_ORDER);
  if (user_stack_page_id < 0) { // Out of memory.
    free_pages(user_program_page_id);
    return false;
  }
  _user_stack_end = (char *)pa_to_kernel_va(page_id_to_pa(user_stack_page_id)) +
                    (1 << USER_STACK_ORDER);

  // Copy the user program.

  memcpy(_user_program_start, start, len);
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

  __asm__ __volatile__("msr elr_el1, %0" : : "r"(_user_program_start));
  __asm__ __volatile__("msr sp_el0, %0" : : "r"(_user_stack_end));
  __asm__ __volatile__("eret");

  __builtin_unreachable();
}
