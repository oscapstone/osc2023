#include "peripherals/reboot.h"

void set(long addr, unsigned int value)
{
    volatile unsigned int *point = (unsigned int *)addr;
    *point = value;
}

void reset(int tick)
{                                                   // reboot after watchdog timer expire
    set(PM_RSTC, (unsigned int)PM_PASSWORD | 0x20); // full reset
    set(PM_WDOG, (unsigned int)PM_PASSWORD | tick); // number of watchdog tick
}

void cancel_reset()
{
    set(PM_RSTC, (unsigned int)PM_PASSWORD | 0); // full reset
    set(PM_WDOG, (unsigned int)PM_PASSWORD | 0); // number of watchdog tick
}