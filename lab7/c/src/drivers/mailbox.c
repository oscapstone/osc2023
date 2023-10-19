#include "oscos/drivers/mailbox.h"

#include <stdalign.h>

#include "oscos/drivers/board.h"
#include "oscos/mem/vm.h"

#define MAILBOX_REG_BASE ((void *)((char *)PERIPHERAL_BASE + 0xb880))

typedef struct {
  volatile uint32_t read_write;
  const volatile uint32_t _reserved[3];
  volatile uint32_t peek;
  volatile uint32_t sender;
  volatile uint32_t status;
  volatile uint32_t config;
} mailbox_reg_t;

#define MAILBOX_REGS ((mailbox_reg_t *)MAILBOX_REG_BASE)

#define MAILBOX_STATUS_EMPTY_MASK ((uint32_t)(1 << 30))
#define MAILBOX_STATUS_FULL_MASK ((uint32_t)(1 << 31))

#define GET_BOARD_REVISION ((uint32_t)0x00010002)
#define GET_ARM_MEMORY ((uint32_t)0x00010005)
#define REQUEST_CODE ((uint32_t)0x00000000)
#define REQUEST_SUCCEED ((uint32_t)0x80000000)
#define REQUEST_FAILED ((uint32_t)0x80000001)
#define TAG_REQUEST_CODE ((uint32_t)0x00000000)
#define END_TAG ((uint32_t)0x00000000)

void mailbox_init(void) {
  // No-op.
}

void mailbox_call(uint32_t message[], const unsigned char channel) {
  PERIPHERAL_WRITE_BARRIER();

  const uint32_t mailbox_write_data = kernel_va_to_pa(message) | channel;

  while (MAILBOX_REGS[1].status & MAILBOX_STATUS_FULL_MASK)
    ;
  MAILBOX_REGS[1].read_write = mailbox_write_data;

  for (;;) {
    while (MAILBOX_REGS[0].status & MAILBOX_STATUS_EMPTY_MASK)
      ;
    const uint32_t mailbox_read_data = MAILBOX_REGS[0].read_write;

    if (mailbox_write_data == mailbox_read_data)
      break;
  }

  PERIPHERAL_READ_BARRIER();
}

uint32_t mailbox_get_board_revision(void) {
  alignas(16) uint32_t mailbox[7] = {7 * sizeof(uint32_t),
                                     REQUEST_CODE,
                                     GET_BOARD_REVISION,
                                     4,
                                     TAG_REQUEST_CODE,
                                     0,
                                     END_TAG};

  mailbox_call(mailbox, MAILBOX_CHANNEL_PROPERTY_TAGS_ARM_TO_VC);

  return mailbox[5];
}

arm_memory_t mailbox_get_arm_memory(void) {
  alignas(16) uint32_t mailbox[8] = {8 * sizeof(uint32_t),
                                     REQUEST_CODE,
                                     GET_ARM_MEMORY,
                                     8,
                                     TAG_REQUEST_CODE,
                                     0,
                                     0,
                                     END_TAG};

  mailbox_call(mailbox, MAILBOX_CHANNEL_PROPERTY_TAGS_ARM_TO_VC);

  return (arm_memory_t){.base = mailbox[5], .size = mailbox[6]};
}

init_framebuffer_result_t mailbox_init_framebuffer(void) {
  alignas(16) uint32_t mailbox[36] = {
      [0] = 35 * 4,   [1] = REQUEST_CODE,

      [2] = 0x48003, // set phy wh
      [3] = 8,        [4] = 8,
      [5] = 1024, // FrameBufferInfo.width
      [6] = 768,  // FrameBufferInfo.height

      [7] = 0x48004, // set virt wh
      [8] = 8,        [9] = 8,
      [10] = 1024, // FrameBufferInfo.virtual_width
      [11] = 768,  // FrameBufferInfo.virtual_height

      [12] = 0x48009, // set virt offset
      [13] = 8,       [14] = 8,
      [15] = 0, // FrameBufferInfo.x_offset
      [16] = 0, // FrameBufferInfo.y.offset

      [17] = 0x48005, // set depth
      [18] = 4,       [19] = 4,
      [20] = 32, // FrameBufferInfo.depth

      [21] = 0x48006, // set pixel order
      [22] = 4,       [23] = 4,
      [24] = 1, // RGB, not BGR preferably

      [25] = 0x40001, // get framebuffer, gets alignment on request
      [26] = 8,       [27] = 8,
      [28] = 4096, // FrameBufferInfo.pointer
      [29] = 0,    // FrameBufferInfo.size

      [30] = 0x40008, // get pitch
      [31] = 4,       [32] = 4,
      [33] = 0, // FrameBufferInfo.pitch

      [34] = END_TAG};

  mailbox_call(mailbox, MAILBOX_CHANNEL_PROPERTY_TAGS_ARM_TO_VC);

  return mailbox[20] == 32 && mailbox[28] != 0
             ? (init_framebuffer_result_t){.width = mailbox[5],
                                           .height = mailbox[6],
                                           .pitch = mailbox[33],
                                           .isrgb = mailbox[24],
                                           .framebuffer_base = pa_to_kernel_va(
                                               mailbox[28] & 0x3fffffff),
                                           .framebuffer_size = mailbox[29]}
             : (init_framebuffer_result_t){.framebuffer_base = NULL};
}
