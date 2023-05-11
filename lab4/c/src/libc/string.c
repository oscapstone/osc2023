#include "oscos/libc/string.h"

__attribute__((used)) int memcmp(const void *const s1, const void *const s2,
                                 const size_t n) {
  const unsigned char *const s1_c = s1, *const s2_c = s2;

  for (size_t i = 0; i < n; i++) {
    const int diff = (int)s1_c[i] - s2_c[i];
    if (diff != 0)
      return diff;
  }

  return 0;
}

__attribute__((used)) void *memset(void *const s, const int c, const size_t n) {
  unsigned char *const s_c = s;

  for (size_t i = 0; i < n; i++) {
    s_c[i] = c;
  }

  return s;
}

static void __memmove_forward(unsigned char *const dest,
                              const unsigned char *const src, const size_t n) {
  for (size_t i = 0; i < n; i++) {
    dest[i] = src[i];
  }
}

static void __memmove_backward(unsigned char *const dest,
                               const unsigned char *const src, const size_t n) {
  for (size_t ip1 = n; ip1 > 0; ip1--) {
    const size_t i = ip1 - 1;
    dest[i] = src[i];
  }
}

__attribute__((used)) void *memcpy(void *const restrict dest,
                                   const void *const restrict src,
                                   const size_t n) {
  __memmove_forward(dest, src, n);
  return dest;
}

__attribute__((used)) void *memmove(void *const dest, const void *const src,
                                    const size_t n) {
  if (dest < src) {
    __memmove_forward(dest, src, n);
  } else if (dest > src) {
    __memmove_backward(dest, src, n);
  }

  return dest;
}

int strcmp(const char *const s1, const char *const s2) {
  for (const unsigned char *c1 = (const unsigned char *)s1,
                           *c2 = (const unsigned char *)s2;
       *c1 || *c2; c1++, c2++) {
    const int diff = (int)*c1 - *c2;
    if (diff != 0)
      return diff;
  }

  return 0;
}

int strncmp(const char *const s1, const char *const s2, const size_t n) {
  size_t i = 0;
  const unsigned char *c1 = (const unsigned char *)s1,
                      *c2 = (const unsigned char *)s2;
  for (; i < n && (*c1 || *c2); i++, c1++, c2++) {
    const int diff = (int)*c1 - *c2;
    if (diff != 0)
      return diff;
  }

  return 0;
}

size_t strlen(const char *s) {
  size_t result = 0;
  for (const char *c = s; *c; c++) {
    result++;
  }
  return result;
}
