#ifndef	_UTILS_H
#define	_UTILS_H
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
extern void delay(unsigned int t);
extern void *memcpy(void *dest, const void *src, unsigned int len);
extern int strcmp(char *a, char *b);
extern int strncmp(char *a, char *b, unsigned n);
extern unsigned strlen(char *s);
extern int strstartswith(char *str, char *prefix);
extern char *my_strtok(char *str, const char *delm);
extern char *strtok(char *str, const char *delm);
extern unsigned long long hex2ull(char *s);
typedef unsigned int reg_t;
typedef unsigned long long size_t;
#endif