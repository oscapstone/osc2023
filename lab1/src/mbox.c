#include "mbox.h"

int mbox_call(uint8_t ch, volatile uint32_t* buffer) {
  // drain stale responses
  while (*MBOX_STATUS & MBOX_STATUS_WR_FULL) {
    asm volatile("nop");
  }

  // send the request
  *MBOX_WRITE = ((uint32_t)((uint64_t)buffer) & MBOX_MASK_DATA) |
                (ch & MBOX_MASK_CHANNEL);

  // wait for the response
  while (*MBOX_STATUS & MBOX_STATUS_RD_EMPTY) {
    asm volatile("nop");
  }

  // validate the response
  uint32_t res = *MBOX_READ;
  if ((uint8_t)(res & MBOX_MASK_CHANNEL) != ch) {
    return -1;
  }

  *buffer = res & MBOX_MASK_DATA;

  return 0;
}

int mbox_get_board_revision(uint32_t* board_rev) {
  MBOX_ALLOC_ALIGNED_MEMORY(struct mbox_msg_get_board_revision, msg);
  MBOX_INIT_MSG(msg);
  MBOX_INIT_TAG(msg, MBOX_TAG_GET_BOARD_REVISION);

  int res = mbox_call(MBOX_CH_PROP, (uint32_t*)msg);
  if (res == -1) {
    return -1;
  }

  *board_rev = msg->msg_tag.body.res.board_rev;

  return 0;
}

int mbox_get_arm_memory(uint32_t* mem_base, uint32_t* mem_size) {
  MBOX_ALLOC_ALIGNED_MEMORY(struct mbox_msg_get_arm_memory, msg);
  MBOX_INIT_MSG(msg);
  MBOX_INIT_TAG(msg, MBOX_TAG_GET_ARM_MEMORY);

  int res = mbox_call(MBOX_CH_PROP, (uint32_t*)msg);
  if (res == -1) {
    return -1;
  }

  *mem_base = msg->msg_tag.body.res.mem_base;
  *mem_size = msg->msg_tag.body.res.mem_size;

  return 0;
}
