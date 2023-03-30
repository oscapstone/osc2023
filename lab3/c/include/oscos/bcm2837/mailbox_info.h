#ifndef OSCOS_BCM2837_MAILBOX_INFO_H
#define OSCOS_BCM2837_MAILBOX_INFO_H

#include <stdint.h>

typedef struct {
  uint32_t base, size;
} arm_memory_t;

uint32_t get_board_revision(void);

arm_memory_t get_arm_memory(void);

#endif
