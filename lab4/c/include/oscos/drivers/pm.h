#ifndef OSCOS_DRIVERS_PM_H
#define OSCOS_DRIVERS_PM_H

#include <stdint.h>
#include <stdnoreturn.h>

void pm_init(void);

void pm_reset(uint32_t tick);
void pm_cancel_reset(void);

noreturn void pm_reboot(void);

#endif
