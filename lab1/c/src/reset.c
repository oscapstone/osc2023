#include "oscos/reset.h"

#include "oscos/bcm2837/pm.h"
#include "oscos/serial.h"

void reset(const uint32_t tick) {
  PM->RSTC = PM_PASSWORD | 0x20;
  PM->WDOG = PM_PASSWORD | tick;
}

void cancel_reset(void) {
  PM->RSTC = PM_PASSWORD | 0;
  PM->WDOG = PM_PASSWORD | 0;
}

noreturn void reboot(void) {
  serial_flush();

  reset(1);
  for (;;)
    ;
}
