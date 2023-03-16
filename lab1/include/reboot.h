#ifndef __REBOOT_H__
#define __REBOOT_H__

#define PM_PASSWORD 0x5A000000
#define PM_RSTC     0x3F10001C
#define PM_WDOG     0x3F100024

void set(long addr, unsigned int value);
void reset(int tick);
void cancel_reset(void);

#endif