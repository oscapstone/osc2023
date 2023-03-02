#ifndef _P_GPIO_H
#define _P_GPIO_H

#include "peripherals/base.h"

#define GPFSEL0 (PBASE + 0x00200000)
#define GPFSEL1 (PBASE + 0x00200004)
#define GPFSEL2 (PBASE + 0x00200008)
#define GPFSEL3 (PBASE + 0x0020000c)
#define GPFSEL4 (PBASE + 0x00200010)
#define GPFSEL5 (PBASE + 0x00200014)

// PBASE + 0x00200018 is reserved

#define GPSET0 (PBASE + 0x0020001c)
#define GPSET1 (PBASE + 0x00200020)

// PBASE + 0x00200024 is reserved

#define GPCLR0 (PBASE + 0x00200028)
#define GPCLR1 (PBASE + 0x0020002c)

// PBASE + 0x00200030 is reserved

#define GPLEV0 (PBASE + 0x00200034)
#define GPLEV1 (PBASE + 0x00200038)

// PBASE + 0x0020003c is reserved

#define GPEDS0 (PBASE + 0x00200040)
#define GPEDS1 (PBASE + 0x00200044)

// PBASE + 0x00200048 is reserved

#define GPREN0 (PBASE + 0x0020004c)
#define GPREN1 (PBASE + 0x00200050)

// PBASE + 0x00200054 is reserved

#define GPFEN0 (PBASE + 0x00200058)
#define GPFEN1 (PBASE + 0x0020005c)

// PBASE + 0x00200060 is reserved

#define GPHEN0 (PBASE + 0x00200064)
#define GPHEN1 (PBASE + 0x00200068)

// PBASE + 0x0020006c is reserved

#define GPLEN0 (PBASE + 0x00200070)
#define GPLEN1 (PBASE + 0x00200064)

// PBASE + 0x00200078 is reserved

#define GPAREN0 (PBASE + 0x0020007c)
#define GPAREN1 (PBASE + 0x00200080)

// PBASE + 0x00200084 is reserved

#define GPAFEN0 (PBASE + 0x00200088)
#define GPAFEN1 (PBASE + 0x0020008c)

// PBASE + 0x00200090 is reserved

#define GPPUD (PBASE + 0x00200094)
#define GPPUDCLK0 (PBASE + 0x00200098)
#define GPPUDCLK1 (PBASE + 0x0020009c)
#endif