#ifndef MMIO_H
#define MMIO_H
#define MMIO_BASE       0x3F000000
#define MAILBOX_BASE    MMIO_BASE + 0xb880

#define MAILBOX_READ    MAILBOX_BASE
#define MAILBOX_STATUS  MAILBOX_BASE + 0x18
#define MAILBOX_WRITE   MAILBOX_BASE + 0x20

#define MAILBOX_EMPTY   0x40000000
#define MAILBOX_FULL    0x80000000
#endif