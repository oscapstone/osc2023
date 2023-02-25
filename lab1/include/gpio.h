#ifndef GPIO_H
#define GPIO_H

#define MMIO_BASE 0x3f000000
#define BUS_BASE 0x7e000000

/* GPIO function select
 * If set to 0 -> default behavior
 * 000 -> input
 * 001 -> output
 * others -> alternative function
 */
#define PHY_GPFSEL0 0x7e200000 /* 9 - 0 */
#define PHY_GPFSEL1 0x7e200004 /* 19 - 10 */
#define PHY_GPFSEL2 0x7e200008
#define PHY_GPFSEL3 0x7e20000c
#define PHY_GPFSEL4 0x7e200010
#define PHY_GPFSEL5 0x7e200014

#define GPFSEL0 (volatile unsigned int*)((PHY_GPFSEL0) - (BUS_BASE) + (MMIO_BASE))
#define GPFSEL1 (volatile unsigned int*)((PHY_GPFSEL1) - (BUS_BASE) + (MMIO_BASE))
#define GPFSEL2 (volatile unsigned int*)((PHY_GPFSEL2) - (BUS_BASE) + (MMIO_BASE))
#define GPFSEL3 (volatile unsigned int*)((PHY_GPFSEL3) - (BUS_BASE) + (MMIO_BASE))
#define GPFSEL4 (volatile unsigned int*)((PHY_GPFSEL4) - (BUS_BASE) + (MMIO_BASE))
#define GPFSEL5 (volatile unsigned int*)((PHY_GPFSEL5) - (BUS_BASE) + (MMIO_BASE))

/* GPIO Pin output Set */
#define PHY_GPSET0 0x7e20001c
#define PHY_GPSET1 0x7e200020

#define GPSET0 ((PHY_GPSET0) - (BUS_BASE) + (MMIO_BASE))
#define GPSET1 ((PHY_GPSET1) - (BUS_BASE) + (MMIO_BASE))

/* GPIO Pin Clear */
#define PHY_GPCLR0 0x7e200028
#define PHY_GPCLR1 0x7e20002c

#define GPCLR0 (volatile unsigned int*)((PHY_GPCLR0) - (BUS_BASE) + (MMIO_BASE))
#define GPCLR1 (volatile unsigned int*)((PHY_GPCLR1) - (BUS_BASE) + (MMIO_BASE))

/* GPIO Pull-up/down Enable 
 * 00 -> disable 
 * 01 -> enable pull-up
 * 10 -> enable pull-down
 * 11 -> reserved
 * */
#define PHY_GPPUD 0x7e200094
#define GPPUD (volatile unsigned int*)((PHY_GPPUD) - (BUS_BASE) + (MMIO_BASE))

/* GPIO Pull-up/down Enable clock 
 * 1 -> sent clk, apply change
 * 0 -> no effect
 * */
#define PHY_GPPUDCLK0 0x7e200098 /* 31 - 0 */
#define PHY_GPPUDCLK1 0x7e20009c /* 53 - 32 */

#define GPPUDCLK0 (volatile unsigned int*)((PHY_GPPUDCLK0) - (BUS_BASE) + (MMIO_BASE))
#define GPPUDCLK1 (volatile unsigned int*)((PHY_GPPUDCLK1) - (BUS_BASE) + (MMIO_BASE))

#endif // GPIO_H
