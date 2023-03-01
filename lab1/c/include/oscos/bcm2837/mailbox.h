#ifndef OSCOS_BCM2837_MAILBOX_H
#define OSCOS_BCM2837_MAILBOX_H

#include <stdint.h>

#include "oscos/bcm2837/peripheral.h"

#define MAILBOX_BASE ((void *)((uintptr_t)PERIPHERAL_BASE + 0xb880))

typedef struct {
  volatile uint32_t read_write;
  const volatile uint32_t _reserved[3];
  volatile uint32_t peek;
  volatile uint32_t sender;
  volatile uint32_t status;
  volatile uint32_t config;
} mailbox_t;

#define MAILBOXES ((mailbox_t *)MAILBOX_BASE)

#define MAILBOX_READ_WRITE_CHAN_PROPERTY_TAGS_ARM_TO_VC ((uint32_t)8)

#define MAILBOX_STATUS_EMPTY_MASK ((uint32_t)0x40000000)
#define MAILBOX_STATUS_FULL_MASK ((uint32_t)0x80000000)

void mailbox_call(uint32_t message[]);

#endif
