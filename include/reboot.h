#ifndef _REBOOT_H
#define _REBOOT_H
#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024
extern void set(long addr, unsigned int value);
extern void reset(int tick);
extern void cancel_reset();
#endif