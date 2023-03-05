#ifndef	_P_GPIO_H
#define	_P_GPIO_H

#define PBASE 0x3F000000
// use phy addr 0x3f instead of bus addr 0x7e
#define GPFSEL1         ((volatile unsigned int*)(PBASE+0x00200004))
#define GPSET0          ((volatile unsigned int*)(PBASE+0x0020001C))
#define GPCLR0          ((volatile unsigned int*)(PBASE+0x00200028))
#define GPPUD           ((volatile unsigned int*)(PBASE+0x00200094))
#define GPPUDCLK0       ((volatile unsigned int*)(PBASE+0x00200098))

#endif