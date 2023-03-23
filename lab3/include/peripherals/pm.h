/*
 * Power Management
 */

#ifndef	PERIPHERALS_PM_H
#define	PERIPHERALS_PM_H

#include "base.h"

#define PM_PASSWORD        0x5a000000
#define PM_RESET_CTRL      (MMIO_BASE + 0x0010001c)
#define PM_WATCHDOG_REG    (MMIO_BASE + 0x00100024)

#endif /* PERIPHERALS_PM_H */