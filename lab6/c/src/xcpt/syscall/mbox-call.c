#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "oscos/console.h"
#include "oscos/drivers/mailbox.h"
#include "oscos/mem/page-alloc.h"
#include "oscos/mem/vm.h"
#include "oscos/sched.h"
#include "oscos/uapi/errno.h"
#include "oscos/utils/critical-section.h"

static bool _is_valid_mbox_ch(const unsigned char ch) { return ch < 10; }

static bool _is_valid_mbox_ptr(const unsigned int *const mbox) {
  return ((uintptr_t)mbox & 0xf) == 0;
}

int sys_mbox_call(const unsigned char ch, unsigned int *const mbox) {
  // Note on return values:
  // The video player treats a return value of 0 as failure and any non-zero
  // return value as success. This is confirmed by reverse-engineering the video
  // player. This system call therefore follows the protocol used by the video
  // player rather than the usual convention since we have to execute the video
  // player properly.

  if (!_is_valid_mbox_ch(ch))
    return /* -EINVAL */ 0;

  if (!_is_valid_mbox_ptr(mbox))
    return /* -EINVAL */ 0;

  process_t *const curr_process = current_thread()->process;

  // CoW-fault the mailbox memory, so that it is not shared with any other
  // address spaces.

  const size_t mbox_buf_len = mbox[0];
  for (size_t offset = 0; offset < mbox_buf_len; offset += 1 << PAGE_ORDER) {
    const vm_map_page_result_t result =
        vm_cow(&curr_process->addr_space, (char *)mbox + offset);
    if (result == VM_MAP_PAGE_SEGV) {
      deliver_signal(curr_process, SIGSEGV);
      return 0;
    } else if (result == VM_MAP_PAGE_NOMEM) {
      // For a lack of better things to do.
      thread_exit();
    }
  }

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  mailbox_call(mbox, ch);

  CRITICAL_SECTION_LEAVE(daif_val);

  return /* 0 */ 1;
}
