#ifndef BCM2835_GPIO
#define BCM2835_GPIO

#include "bcm2835/addr.h"

/** GPIO Function Select Register
 */

#define GPFSEL1 PRPHRL(0x200004U)

/** GPIO Pull-up/down Register
 *
 *   Bit(s) Description
 *   31-02  Unused
 *   01-00  00 = Disable pull-up/down
 *          01 = Enable Pull Down control
 *          10 = Enable Pull Up control
 *          11 = Reserved 
 */

#define GPPUD PRPHRL(0x200094U)

/** GPIO Pin Pull-up/down Enable Clock
 */

#define GPPUDCLK0 PRPHRL(0x200098U)

#endif