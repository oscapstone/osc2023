#include "power.h"
#include "utils.h"


void reset(int tick) {                 // reboot after watchdog timer expire
    write_reg_32(PM_RSTC, PM_PASSWORD | 0x20);  // full reset
    write_reg_32(PM_WDOG, PM_PASSWORD | tick);  // number of watchdog tick
}