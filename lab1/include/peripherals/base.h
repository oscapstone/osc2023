#ifndef __PERIPHERALS_BASE_H__
#define __PERIPHERALS_BASE_H__

#ifdef RPI4
#define MMIO_BASE 0xFE000000
#elif RPI3
#define MMIO_BASE 0x3F000000
#endif

#endif
