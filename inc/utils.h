#ifndef	_UTILS_H
#define	_UTILS_H

#include <type.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#define enable_interrupt(){           \
    asm volatile("msr DAIFClr, 0xf"); \
}

#define disable_interrupt(){           \
    asm volatile("msr DAIFSet, 0xf"); \
}

#define get_elem_idx(elem, array) \
    (((char *)elem - (char *)array) / sizeof(array[0]))

#define read_sysreg(r) ({                       \
    uint64 __val;                               \
    asm volatile("mrs %0, " #r : "=r" (__val)); \
    __val;                                      \
})

#define write_sysreg(r, v) {                \
    uint64 __val = (uint64)(v);             \
    asm volatile("msr " #r ", %x0"          \
             : : "rZ" (__val));             \
}

extern void delay ( unsigned long);
extern void put32 ( unsigned long, unsigned int );
extern unsigned int get32 ( unsigned long );

static inline uint32 save_and_disable_interrupt(void){
    uint32 daif;
    daif = read_sysreg(DAIF);
    disable_interrupt();
    return daif;
}

static inline void restore_interrupt(uint32 daif){
    write_sysreg(DAIF, daif);
}

#endif  /*_UTILS_H */