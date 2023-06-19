#ifndef	_UTILS_H
#define	_UTILS_H

#include <type.h>

#define PA2VA(x) (((uint64)(x)) | 0xffff000000000000)
#define VA2PA(x) (((uint64)(x)) & 0x0000ffffffffffff)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#define enable_interrupt(){           \
    asm volatile("msr DAIFClr, 0xf"); \
}

#define disable_interrupt(){           \
    asm volatile("msr DAIFSet, 0xf"); \
}

#define set_page_table(task) do {               \
    asm volatile(                               \
        "mov x9, %0\n"                          \
        "and x9, x9, #0x0000ffffffffffff\n"     \
        "dsb ish\n"                             \
        "msr ttbr0_el1, x9\n"                   \
        "tlbi vmalle1is\n"                      \
        "dsb ish\n"                             \
        "isb\n"                                 \
        :: "r" (task->page_table)               \
    );                                          \
} while (0)

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