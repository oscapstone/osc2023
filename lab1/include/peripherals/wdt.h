#ifndef __PERIPHERALS_WDT_H__
#define __PERIPHERALS_WDT_H__

#include <stdint.h>

#include "peripherals/base.h"

// clang-format off
#ifdef RPI4
#define PM_BASE
#elif RPI3
#define PM_BASE (MMIO_BASE + 0x100000)
#endif

// addresses
#define PM_RSTC ((volatile uint32_t*)(PM_BASE + 0x1C))
#define PM_RSTS ((volatile uint32_t*)(PM_BASE + 0x20))
#define PM_WDOG ((volatile uint32_t*)(PM_BASE + 0x24))
// clang-format on

#endif
