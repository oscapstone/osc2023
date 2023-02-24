#ifndef	_UTILS_H
#define	_UTILS_H
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
extern void delay(unsigned int t);
extern void *memcpy(void *dest, const void *src, unsigned int len);
extern int strcmp(char *a, char *b);
#endif