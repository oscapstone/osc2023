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
  if (!_is_valid_mbox_ch(ch))
    return -EINVAL;

  if (!_is_valid_mbox_ptr(mbox))
    return -EINVAL;

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  mailbox_call(mbox, ch);

  CRITICAL_SECTION_LEAVE(daif_val);

  return 0;
}
