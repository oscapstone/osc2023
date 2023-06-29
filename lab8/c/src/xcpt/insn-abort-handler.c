#include "oscos/console.h"
#include "oscos/panic.h"
#include "oscos/sched.h"

void xcpt_default_handler(uint64_t vten);

void xcpt_insn_abort_handler(const uint64_t esr_val) {
  thread_t *const curr_thread = current_thread();
  process_t *const curr_process = curr_thread->process;

  void *fault_addr;
  __asm__ __volatile__("mrs %0, far_el1" : "=r"(fault_addr));

  const uint64_t ifsc = esr_val & ((1 << 6) - 1);

  if (ifsc >> 2 == 0x1) { // Translation fault.
    const vm_map_page_result_t result =
        vm_map_page(&curr_process->addr_space, fault_addr);
    if (result == VM_MAP_PAGE_SEGV) {
#ifdef VM_ENABLE_DEBUG_LOG
      console_printf("DEBUG: vm: Segmentation fault, PID %zu, address 0x%p\n",
                     curr_process->id, fault_addr);
#endif
      deliver_signal(curr_process, SIGSEGV);
    } else if (result == VM_MAP_PAGE_NOMEM) {
      // For a lack of better things to do.
      thread_exit();
    } else {
#ifdef VM_ENABLE_DEBUG_LOG
      console_printf("DEBUG: vm: Translation fault, PID %zu, address 0x%p\n",
                     curr_process->id, fault_addr);
#endif
    }
  } else if (ifsc >> 2 == 0x3) { // Permission fault.
    const vm_map_page_result_t result =
        vm_cow(&curr_process->addr_space, fault_addr);
    if (result == VM_MAP_PAGE_SEGV) {
#ifdef VM_ENABLE_DEBUG_LOG
      console_printf("DEBUG: vm: Segmentation fault, PID %zu, address 0x%p\n",
                     curr_process->id, fault_addr);
#endif
      deliver_signal(curr_process, SIGSEGV);
    } else if (result == VM_MAP_PAGE_NOMEM) {
      // For a lack of better things to do.
      thread_exit();
    } else {
#ifdef VM_ENABLE_DEBUG_LOG
      console_printf("DEBUG: vm: CoW fault, PID %zu, address 0x%p\n",
                     curr_process->id, fault_addr);
#endif
    }
  } else {
    xcpt_default_handler(8);
  }
}
