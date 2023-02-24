#ifndef _HARDWARE_INFO_H
#define _HARDWARE_INFO_H
#define GET_FIRMWARE_VERSION    0x00000001
#define GET_BOARD_MODEL         0x00010001
#define GET_BOARD_REVISION      0x00010002
#define GET_BOARD_MAC_ADDRESS   0x00010003
#define GET_BOARD_SERIAL        0x00010004
#define GET_ARM_MEMORY          0x00010005
#define GET_VC_MEMORY           0x00010006
#define GET_CLOCKS              0x00010007
#define GET_TEMPERATURE         0x00030006
#define GET_MAX_CLOCK_RATE      0x00030004
#define SET_CLOCK_RATE          0x00038002
#define GET_MAX_TEMPERATURE     0x00030020
#define GET_MIN_TEMPERATURE     0x00030021
#define LOCK_MEMORY             0x0003000f
#define UNLOCK_MEMORY           0x00030010
#define RELEASE_MEMORY          0x00030011
#define GET_MAC_ADDRESS         0x00010003
#define GET_MIN_CLOCK_RATE      0X00030007
#define GET_CLOCK_RATE          0X00030002
#define GET_VOLTAGE             0X0003000a
#define SET_VOLTAGE             0x0003800b
#define GET_MAX_VOLTAGE         0x0003000c
#define GET_MIN_VOLTAGE         0x0003000e
#define ALLOCATE_MEMORY         0x0003000d
#define REQUEST_CODE            0x00000000
#define REQUEST_SUCCEED         0x80000000
#define REQUEST_FAILED          0x80000001
#define TAG_REQUEST_CODE        0x00000000
#define END_TAG                 0x00000000
extern void get_hw_info(unsigned int tag_identifier);
#endif