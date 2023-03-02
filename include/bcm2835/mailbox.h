#ifndef BCM2835_MAILBOX
#define BCM2835_MAILBOX

#include "bcm2835/addr.h"

#define MAILBOX_BASE    0xB880U

#define MAILBOX_READ    PRPHRL(MAILBOX_BASE + 0x00U)
#define MAILBOX_STATUS  PRPHRL(MAILBOX_BASE + 0x18U)
#define MAILBOX_WRITE   PRPHRL(MAILBOX_BASE + 0x20U)

// This bit is set in the status register if there is no space to write into the mailbox
#define MBOX_FULL       0x80000000U
// This bit is set in the status register if there is nothing to read from the mailbox
#define MBOX_EMPTY      0x40000000U

#define MB_CH_ARM2VC    0x00000008U

#define MB_TAG_START    0x00000000U
#define MB_RESP_OK      0x80000000U
#define MB_RESP_ERROR   0x80000001U
#define MB_TAG_END      0x00000000U

#define MB_TAG_BOARD_REVISION   0x00010002U
#define MB_TAG_ARM_MEMORY       0x00010005U

extern unsigned int mb_buf[36];

void mb_request(unsigned int ch);

#endif