#ifndef _MM_H
#define _MM_H

#ifndef __ASSEMBLER__
void memzero(unsigned long src, unsigned long n);
void memncpy(char *dst, char *src, unsigned long n);

#endif

#endif  /* _MM_H */