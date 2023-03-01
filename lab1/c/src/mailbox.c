#include "oscos/bcm2837/mailbox.h"

#include "oscos/bcm2837/peripheral_memory_barrier.h"

void mailbox_call(uint32_t message[]) {
  PERIPHERAL_WRITE_BARRIER();

  const uint32_t mailbox_write_data =
      (uint32_t)(uintptr_t)message |
      MAILBOX_READ_WRITE_CHAN_PROPERTY_TAGS_ARM_TO_VC;

  while (MAILBOXES[0].status & MAILBOX_STATUS_FULL_MASK)
    ;
  MAILBOXES[1].read_write = mailbox_write_data;

  for (;;) {
    while (MAILBOXES[0].status & MAILBOX_STATUS_FULL_MASK)
      ;
    const uint32_t mailbox_read_data = MAILBOXES[0].read_write;

    if (mailbox_write_data == mailbox_read_data)
      break;
  }

  PERIPHERAL_READ_BARRIER();
}
