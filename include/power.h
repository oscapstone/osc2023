#ifndef __POWER_H
#define __POWER_H
#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024

void reset(int tick);
#endif