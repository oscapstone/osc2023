#ifndef _BOOT_H
#define _BOOT_H

extern void delay(unsigned long);
extern void put32(unsigned long, unsigned int);
extern unsigned int get32(unsigned long);
extern void put64(unsigned long, unsigned int);
extern unsigned int get64(unsigned long);

#define read_sysreg(r) ({        \
    unsigned long __val;         \
    asm volatile("mrs %0, " #r   \
                 : "=r"(__val)); \
    __val;                       \
})

#define write_sysreg(r, __val) ({                \
    asm volatile("msr " #r ", %0" ::"r"(__val)); \
})

#define read_gen_reg(r) ({       \
    unsigned long __val;         \
    asm volatile("mov %0, " #r   \
                 : "=r"(__val)); \
    __val;                       \
})

#define write_gen_reg(r, __val) ({               \
    asm volatile("mov " #r ", %0" ::"r"(__val)); \
})

#endif /*_BOOT_H */
