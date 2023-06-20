#ifndef _MM_H
#define _MM_H

#ifndef __ASSEMBLER__
void memzero(char* src, unsigned long n);
void memncpy(char *dst, char *src, unsigned long n);

void mm_init(char *fdt_base);
void *kmalloc(int size);
void kfree(void *ptr);

#endif

#endif  /* _MM_H */