#ifndef REBOOT_H

#define REBOOT_H
#define PM_PASSWORD 	0x5A000000
#define PM_RSTC 		(unsigned int*)0xFFFF00003F10001C
#define PM_WDOG 		(unsigned int*)0xFFFF00003F100024
#define PM_SET			0x00028001

#endif

void power_off();
void reset(int tick);
void cancel_reset();
