#ifndef BCM2835_ADDR
#define BCM2835_ADDR

#define PRPHRL_BASE 0x3F000000U
#define PRPHRL(off) ((unsigned int *)(PRPHRL_BASE + off))

#endif