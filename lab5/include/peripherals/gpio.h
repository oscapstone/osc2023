#ifndef	PERIPHERALS_GPIO_H
#define	PERIPHERALS_GPIO_H

#include "peripherals/base.h"

#define GPFSEL1         (MMIO_BASE + 0x00200004)
#define GPSET0          (MMIO_BASE + 0x0020001C)
#define GPCLR0          (MMIO_BASE + 0x00200028)
#define GPPUD           (MMIO_BASE + 0x00200094)
#define GPPUDCLK0       (MMIO_BASE + 0x00200098)

#endif /* PERIPHERALS_GPIO_H */