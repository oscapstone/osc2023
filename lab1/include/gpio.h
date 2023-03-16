#ifndef _GPIO_H
#define _GPIO_H

#define MMIO_BASE   0x3F000000

#define GPFSEL1     ((volatile unsigned int *)(MMIO_BASE+0x00200004)) //af GPIO 14,15
//#define GPSET0      ((volatile unsigned int *)(MMIO_BASE+0x0020001C)) //output set 0 pin 0-31 
//#define GPCLR0      ((volatile unsigned int *)(MMIO_BASE+0x00200028)) //pin output clear 0
#define GPPUD       ((volatile unsigned int *)(MMIO_BASE+0x00200094)) // pin pull-up/down enable
#define GPPUDCLK0   ((volatile unsigned int *)(MMIO_BASE+0x00200098)) // pin pull-up/down enable clk 0




#endif 