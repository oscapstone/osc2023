#include <stdbool.h>
#include <stdint.h>

#include "oscos/drivers/mailbox.h"
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

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  mailbox_call(mbox, ch);

  CRITICAL_SECTION_LEAVE(daif_val);

  return /* 0 */ 1;
}
