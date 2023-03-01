#ifndef __PERIPHERALS_MBOX_H__
#define __PERIPHERALS_MBOX_H__

#include "peripherals/base.h"

// clang-format off
#ifdef RPI4
#define MBOX_BASE
#elif RPI3
#define MBOX_BASE (MMIO_BASE + 0xB880)
#endif

// addresses
#define MBOX_READ   ((volatile uint32_t*)(MBOX_BASE + 0x00))
#define MBOX_POLL   ((volatile uint32_t*)(MBOX_BASE + 0x10))
#define MBOX_SENDER ((volatile uint32_t*)(MBOX_BASE + 0x14))
#define MBOX_STATUS ((volatile uint32_t*)(MBOX_BASE + 0x18))
#define MBOX_CONFIG ((volatile uint32_t*)(MBOX_BASE + 0x1C))
#define MBOX_WRITE  ((volatile uint32_t*)(MBOX_BASE + 0x20))
// clang-format on

#endif
