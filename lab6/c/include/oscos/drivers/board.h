#ifndef OSCOS_DRIVERS_BOARD_H
#define OSCOS_DRIVERS_BOARD_H

// ARM physical address. See
// https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#peripheral-addresses
#define PERIPHERAL_BASE ((void *)0x3f000000)

#define ARM_LOCAL_PERIPHERAL_BASE ((void *)0x40000000)

// ? Is the shareability domain for the peripheral memory barriers correct?

#define PERIPHERAL_READ_BARRIER()                                              \
  __asm__ __volatile__("dsb nshld" : : : "memory")
#define PERIPHERAL_WRITE_BARRIER()                                             \
  __asm__ __volatile__("dsb nshst" : : : "memory")

#endif
