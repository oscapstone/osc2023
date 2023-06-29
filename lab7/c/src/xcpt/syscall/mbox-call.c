#include <stdbool.h>
#include <stdint.h>

#include "oscos/drivers/mailbox.h"
#include "oscos/libc/string.h"
#include "oscos/mem/malloc.h"
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

  const size_t mbox_len = mbox[0];
  uint32_t *const mbox_kernel = malloc(mbox_len);
  memcpy(mbox_kernel, mbox, mbox_len);

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  mailbox_call(mbox_kernel, ch);

  CRITICAL_SECTION_LEAVE(daif_val);

  memcpy(mbox, mbox_kernel, mbox_len);
  free(mbox_kernel);

  return /* 0 */ 1;
}
