/* a properly aligned buffer */
/* use this buffer(global variable) directly and the mbox_call will use it after call*/ 
/* mbox  format https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface */
/* mbox address need to be aligned to 16 bytes */
extern volatile unsigned int mbox[36];

#define MBOX_REQUEST    0

/* channels */
#define MBOX_CH_POWER   0
#define MBOX_CH_FB      1
#define MBOX_CH_VUART   2
#define MBOX_CH_VCHIQ   3
#define MBOX_CH_LEDS    4
#define MBOX_CH_BTNS    5
#define MBOX_CH_TOUCH   6
#define MBOX_CH_COUNT   7
#define MBOX_CH_PROP    8

/* tags */
#define GET_BOARD_REVISION      0x10002
#define MBOX_TAG_GETSERIAL      0x10004
#define GET_ARM_MEMORY          0x10005
#define MBOX_TAG_LAST           0

int mbox_call(unsigned char ch);