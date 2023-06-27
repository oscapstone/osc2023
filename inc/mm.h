#ifndef _MM_H
#define _MM_H

#ifndef __ASSEMBLER__

#include <type.h>
void memzero(char* src, unsigned long n);
void memncpy(char *dst, const char *src, unsigned long n);
void memset(void *ptr, uint8 value, uint64 bum);

void mm_init(char *fdt_base);
void *kmalloc(int size);
void kfree(void *ptr);

#endif

#endif  /* _MM_H */