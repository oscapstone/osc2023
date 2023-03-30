#include "oscos/libc/string.h"

int strcmp(const char *const s1, const char *const s2) {
  const char *c1 = s1, *c2 = s2;

  for (; *c1 && *c2; c1++, c2++) {
    if (*c1 < *c2)
      return -1;
    if (*c1 > *c2)
      return 1;
  }

  if (*c2)
    return -1;
  if (*c1)
    return 1;
  return 0;
}

int strncmp(const char *const s1, const char *const s2, const size_t n) {
  const char *c1 = s1, *c2 = s2;
  size_t n_compared = 0;

  for (; *c1 && *c2 && n_compared < n; c1++, c2++, n_compared++) {
    if (*c1 < *c2)
      return -1;
    if (*c1 > *c2)
      return 1;
  }

  if (n_compared == n)
    return 0;
  if (*c2)
    return -1;
  if (*c1)
    return 1;
  return 0;
}

size_t strlen(const char *const s) {
  size_t result = 0;
  for (const char *c = s; *c; c++) {
    result++;
  }
  return result;
}

void *memcpy(void *const restrict dest, const void *const restrict src,
             const size_t n) {
  char *restrict dc = dest;
  const char *restrict sc = src;
  for (size_t i = 0; i < n; i++) {
    *dc++ = *sc++;
  }

  return dest;
}

void *memset(void *s, int c, size_t n) {
  char *const p = s;
  for (size_t i = 0; i < n; i++) {
    p[i] = c;
  }
  return s;
}
