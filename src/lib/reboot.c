#include <reboot.h>
#include <utils.h>

void set(long addr, unsigned int value) {
    volatile unsigned int *point = (unsigned int*)addr;
    *point = value;
}

void reset(int tick) {                 // reboot after watchdog timer expire
    set(PA2VA(PM_RSTC), PM_PASSWORD | 0x20);  // full reset
    set(PA2VA(PM_WDOG), PM_PASSWORD | tick);  // number of watchdog tick

    // Never return
    while(1) {}
}

void cancel_reset() {
    set(PA2VA(PM_RSTC), PM_PASSWORD | 0);  // full reset
    set(PA2VA(PM_WDOG), PM_PASSWORD | 0);  // number of watchdog tick
}