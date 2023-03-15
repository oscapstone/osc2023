#ifndef _TYPES_H
#define _TYPES_H

typedef unsigned long long int uint64;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef long long int int64;
typedef int int32;
typedef short int16;
typedef char int8;

typedef uint32 size_t;
typedef uint8 uint8_t;
typedef uint32 fdt32_t;
typedef uint64 fdt64_t;

#define ALIGN(num, base) ((num + base - 1) & ~(base - 1))
#define NULL 0

#endif /* _TYPES_H */