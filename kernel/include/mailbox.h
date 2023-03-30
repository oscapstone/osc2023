// Mailbox
#define MMIO_BASE 0x3f000000
#define MAILBOX_BASE MMIO_BASE + 0xb880

#define MAILBOX_READ ((volatile unsigned int *)(MAILBOX_BASE))
#define MAILBOX_STATUS ((volatile unsigned int *)(MAILBOX_BASE + 0x18))
#define MAILBOX_WRITE ((volatile unsigned int *)(MAILBOX_BASE + 0x20))

#define MAILBOX_EMPTY 0x40000000
#define MAILBOX_FULL 0x80000000

// Tag
#define GET_BOARD_REVISION 0x00010002
#define GET_ARM_MEMORY 0x00010005

#define REQUEST_CODE 0x00000000
#define REQUEST_SUCCEED 0x80000000
#define REQUEST_FAILED 0x80000001
#define TAG_REQUEST_CODE 0x00000000
#define END_TAG 0x00000000

// Channel
#define MAILBOX_CH_POWER 0
#define MAILBOX_CH_FB 1
#define MAILBOX_CH_VUART 2
#define MAILBOX_CH_VCHIQ 3
#define MAILBOX_CH_LEDS 4
#define MAILBOX_CH_BTNS 5
#define MAILBOX_CH_TOUCH 6
#define MAILBOX_CH_ARM_TO_VC_PROP 8
#define MAILBOX_CH_VC_TO_ARM_PROP 9

void print_board_revision();
void print_arm_memory();
void mailbox_call(unsigned int *mailbox, unsigned char channel);