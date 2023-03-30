#include "oscos/reset.h"

#include "oscos/bcm2837/peripheral_memory_barrier.h"
#include "oscos/bcm2837/pm.h"
#include "oscos/serial.h"

void reset(const uint32_t tick) {
  PERIPHERAL_WRITE_BARRIER();

  PM->RSTC = PM_PASSWORD | 0x20;
  PM->WDOG = PM_PASSWORD | tick;

  PERIPHERAL_READ_BARRIER();
}

void cancel_reset(void) {
  PERIPHERAL_WRITE_BARRIER();

  PM->RSTC = PM_PASSWORD | 0;
  PM->WDOG = PM_PASSWORD | 0;

  PERIPHERAL_READ_BARRIER();
}

noreturn void reboot(void) {
  serial_flush();

  reset(1);
  for (;;)
    ;
}
