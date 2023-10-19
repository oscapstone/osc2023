#ifndef OSCOS_LIBC_STRING_H
#define OSCOS_LIBC_STRING_H

#include <stddef.h>

int memcmp(const void *s1, const void *s2, size_t n);
void *memset(void *s, int c, size_t n);
void *memcpy(void *restrict dest, const void *restrict src, size_t n);
void *memmove(void *dest, const void *src, size_t n);

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

size_t strlen(const char *s);

char *strdup(const char *s);
char *strndup(const char *s, size_t n);

char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);

char *strcpy(char *restrict dst, const char *restrict src);

// Extensions.

/// \brief Swap two non-overlapping blocks of memory.
///
/// \param xs The pointer to the beginning of the first block of memory.
/// \param ys The pointer to the beginning of the second block of memory.
/// \param n The size of the memory blocks.
void memswp(void *restrict xs, void *restrict ys, size_t n);

#endif
