#ifndef	_UTILS_H
#define	_UTILS_H
#ifndef __ASSEMBLER__
#include "stddef.h"
#define NULL (void *)0
#define ALIGN(num, base) ((num + base - 1) & ~(base - 1))
#define max(x, y) ({ \
    typeof(x) _x = (x); \
    typeof(y) _y = (y); \
    (void) (&_x == &_y); \
    _x > _y ? _x : _y; })
#define min(x, y) ({ \
    typeof(x) _x = (x); \
    typeof(y) _y = (y); \
    (void) (&_x == &_y); \
    _x < _y ? _x : _y; })
#define read_sysreg(r) ({                       \
    unsigned long __val;                        \
    asm volatile("mrs %0, " #r : "=r" (__val)); \
    __val;                                      \
})
#define write_sysreg(r, __val) ({                  \
	asm volatile("msr " #r ", %0" :: "r" (__val)); \
})
#define ceil(numerator, denominator) (1 + ((numerator)-1) / (denominator))
#define SWAP(x, y) do { typeof(x) temp##x##y = x; x = y; y = temp##x##y; } while (0)
#define GETBIT(data, k) ( (data & (1 << (k))) >> (k) )
#define SETBIT(data, k) ( (data |= (1 << (k))) )
#define CLRBIT(data, k) ( (data &= ~(1 << (k))) )
#define BUILD_BUG_ON_ZERO(e) (sizeof(struct { int:-!!(e); }))
#define __same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))
/* &a[0] degrades to a pointer: a different type from an array */
#define __must_be_array(a)    BUILD_BUG_ON_ZERO(__same_type((a), &(a)[0]))
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))
// #define offsetof(s,m) __builtin_offsetof(s,m)
// #define container_of(ptr, type, member) ({ \
//     void *__mptr = (void *)(ptr);          \
//     ((type *)(__mptr - offsetof(type, member))); })
extern void delay(unsigned int t);
extern void *memcpy(void *dest, const void *src, unsigned int len);
extern int strcmp(char *a, char *b);
extern int strncmp(char *a, char *b, unsigned n);
extern unsigned strlen(char *s);
extern int strstartswith(char *str, char *prefix);
extern char *my_strtok(char *str, const char *delm);
extern char *strtok(char *str, const char *delm);
extern unsigned long long hex2ull(char *s);
extern unsigned long long alignToNextPowerOf2(unsigned long long num);
extern unsigned ul_log2(unsigned long long n);
typedef unsigned int reg_t;
#endif
#endif