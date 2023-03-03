#ifndef __MBOX_H__
#define __MBOX_H__

#include <stdint.h>

#include "lib.h"
#include "peripherals/mbox.h"

// clang-format off
// channels
#define MBOX_CH_POWER 0 // power management
#define MBOX_CH_FB    1 // framebuffer
#define MBOX_CH_VUART 2 // virtual UART
#define MBOX_CH_VCHIQ 3 // VCHIQ
#define MBOX_CH_LEDS  4 // LEDs
#define MBOX_CH_BTNS  5 // buttons
#define MBOX_CH_TOUCH 6 // touch screen
#define MBOX_CH_COUNT 7 // count
#define MBOX_CH_PROP  8 // property tags: ARM -> VC

// status
#define MBOX_STATUS_WR_FULL  0x80000000
#define MBOX_STATUS_RD_EMPTY 0x40000000

// tags
// ref: https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
#define MBOX_TAG_GET_BOARD_REVISION 0x00010002
#define MBOX_TAG_GET_ARM_MEMORY     0x00010005

#define MBOX_MASK_CHANNEL 0xF
#define MBOX_MASK_DATA    (~MBOX_MASK_CHANNEL)
// clang-format on

#define MBOX_ALLOC_ALIGNED_MEMORY(type, var)                            \
  volatile uint32_t __attribute__((aligned(16))) __##var[sizeof(type)]; \
  type *var = (type *)__##var;

#define MBOX_INIT_MSG(var)              \
  my_memset((var), 0, sizeof(*(var)));  \
  (var)->hdr.buf_size = sizeof(*(var)); \
  (var)->hdr.code = 0;                  \
  (var)->end_tag = 0;

#define MBOX_INIT_TAG(var, tag_id)                                   \
  (var)->msg_tag.tag_hdr.tag = tag_id;                               \
  (var)->msg_tag.tag_hdr.val_buf_size = sizeof((var)->msg_tag.body); \
  (var)->msg_tag.tag_hdr.type = 0;

struct mbox_header {
  /* buffer size in bytes */
  uint32_t buf_size;

  /* buffer request/response code */
  uint32_t code;
};

struct mbox_tag_header {
  /* 28MSB + 4LSB */
  uint32_t tag;

  /* value buffer size in bytes */
  uint32_t val_buf_size;

  /* request/response code */
  uint32_t type : 1;
  uint32_t val_len : 31;
};

struct mbox_msg_get_board_revision {
  struct mbox_header hdr;
  struct {
    struct mbox_tag_header tag_hdr;
    union {
      struct {
      } req;
      struct {
        uint32_t board_rev;
      } res;
    } body;
  } msg_tag;
  uint32_t end_tag;
};

struct mbox_msg_get_arm_memory {
  struct mbox_header hdr;
  struct {
    struct mbox_tag_header tag_hdr;
    union {
      struct {
      } req;
      struct {
        uint32_t mem_base;
        uint32_t mem_size;
      } res;
    } body;
  } msg_tag;
  uint32_t end_tag;
};

int mbox_call(uint8_t, volatile uint32_t *);

int mbox_get_board_revision(uint32_t *);
int mbox_get_arm_memory(uint32_t *, uint32_t *);

#endif
