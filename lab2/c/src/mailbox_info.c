#include "oscos/bcm2837/mailbox_info.h"

#include <stdalign.h>

#include "oscos/bcm2837/mailbox.h"

#define GET_BOARD_REVISION 0x00010002
#define GET_ARM_MEMORY 0x00010005
#define REQUEST_CODE 0x00000000
#define REQUEST_SUCCEED 0x80000000
#define REQUEST_FAILED 0x80000001
#define TAG_REQUEST_CODE 0x00000000
#define END_TAG 0x00000000

uint32_t get_board_revision(void) {
  alignas(16) uint32_t mailbox[7];
  mailbox[0] = 7 * sizeof(uint32_t);
  mailbox[1] = REQUEST_CODE;
  mailbox[2] = GET_BOARD_REVISION;
  mailbox[3] = 4;
  mailbox[4] = TAG_REQUEST_CODE;
  mailbox[5] = 0;
  mailbox[6] = END_TAG;

  mailbox_call(mailbox);

  return mailbox[5];
}

arm_memory_t get_arm_memory(void) {
  alignas(16) uint32_t mailbox[8];
  mailbox[0] = 8 * sizeof(uint32_t);
  mailbox[1] = REQUEST_CODE;
  mailbox[2] = GET_ARM_MEMORY;
  mailbox[3] = 8;
  mailbox[4] = TAG_REQUEST_CODE;
  mailbox[5] = 0;
  mailbox[6] = 0;
  mailbox[7] = END_TAG;

  mailbox_call(mailbox);

  return (arm_memory_t){.base = mailbox[5], .size = mailbox[6]};
}
