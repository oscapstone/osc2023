#ifndef __WDT_H__
#define __WDT_H__

#include "peripherals/wdt.h"

// clang-format off
#define PM_PASSWORD              0x5A000000
#define PM_WDOG_TIME_SET         0x000FFFFF
#define PM_RSTC_WRCFG_CLR        0xFFFFFFCF
#define PM_RSTS_HADWRH_SET       0x00000040
#define PM_RSTC_WRCFG_SET        0x00000030
#define PM_RSTC_WRCFG_FULL_RESET 0x00000020
#define PM_RSTC_RESET            0x00000102
// clang-format on

// timeout ~150us
#define RESTART_TICK 10

void wdt_restart();

#endif
