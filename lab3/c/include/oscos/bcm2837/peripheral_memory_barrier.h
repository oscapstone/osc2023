#ifndef OSCOS_BCM2837_PERIPHERAL_MEMORY_BARRIER_H
#define OSCOS_BCM2837_PERIPHERAL_MEMORY_BARRIER_H

// ? Is the shareability domain correct?

#define PERIPHERAL_READ_BARRIER() __asm__ __volatile__("dsb nshld" ::: "memory")
#define PERIPHERAL_WRITE_BARRIER()                                             \
  __asm__ __volatile__("dsb nshst" ::: "memory")

#endif
