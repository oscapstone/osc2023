#include "peripherals/pm.h"
#include "reboot.h"
#include "utils.h"

/*
 * Reboots after watchdog timer expire
 */
void reset(int n_tick)
{
    put32(PM_RESET_CTRL, PM_PASSWORD | 0x20);      // full reset
    put32(PM_WATCHDOG_REG, PM_PASSWORD | n_tick);  // number of watchdog tick
}

void cancel_reset(void)
{
    put32(PM_RESET_CTRL, PM_PASSWORD | 0);    // full reset
    put32(PM_WATCHDOG_REG, PM_PASSWORD | 0);  // number of watchdog tick
}