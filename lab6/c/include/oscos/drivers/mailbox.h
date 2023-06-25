#ifndef OSCOS_DRIVERS_MAILBOX_H
#define OSCOS_DRIVERS_MAILBOX_H

#include <stdint.h>

#define MAILBOX_CHANNEL_PROPERTY_TAGS_ARM_TO_VC ((unsigned char)8)

typedef struct {
  uint32_t base, size;
} arm_memory_t;

void mailbox_init(void);

void mailbox_call(uint32_t message[], unsigned char channel);

uint32_t mailbox_get_board_revision(void);
arm_memory_t mailbox_get_arm_memory(void);

#endif
