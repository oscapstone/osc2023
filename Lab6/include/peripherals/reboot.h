#ifndef _P_REBOOT_H
#define _P_REBOOT_H

#include "virtual_mem.h"

#define PM_PASSWORD KERNEL_PA_TO_VA(0x5a000000)
#define PM_RSTC KERNEL_PA_TO_VA(0x3F10001c)
#define PM_WDOG KERNEL_PA_TO_VA(0x3F100024)

#endif /*_P_REBOOT_H */