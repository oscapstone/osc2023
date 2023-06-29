#include "oscos/libc/string.h"

#include "oscos/mem/malloc.h"

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

char *strdup(const char *s) {
  const size_t len = strlen(s);

  char *const result = malloc(len + 1);
  if (!result)
    return NULL;

  memcpy(result, s, len);
  result[len] = '\0';

  return result;
}

char *strndup(const char *const s, const size_t n) {
  size_t len = 0;
  for (const char *c = s; len < n && *c; c++) {
    len++;
  }

  char *const result = malloc(len + 1);
  if (!result)
    return NULL;

  memcpy(result, s, len);
  result[len] = '\0';

  return result;
}

char *strchr(const char *const s, const int c) {
  for (const char *p = s; *p; p++) {
    if (*p == c)
      return (char *)p;
  }
  return NULL;
}

char *strrchr(const char *const s, const int c) {
  for (const char *pp1 = s + strlen(s); pp1 != s; pp1--) {
    const char *const p = pp1 - 1;
    if (*p == c)
      return (char *)p;
  }
  return NULL;
}

char *strcpy(char *const restrict dst, const char *const restrict src) {
  char *restrict cd = dst;
  const char *restrict cs = src;
  while (*cs) {
    *cd++ = *cs++;
  }

  return dst;
}

void memswp(void *const restrict xs, void *const restrict ys, const size_t n) {
  unsigned char *const restrict xs_c = xs, *const restrict ys_c = ys;
  for (size_t i = 0; i < n; i++) {
    const unsigned char tmp = xs_c[i];
    xs_c[i] = ys_c[i];
    ys_c[i] = tmp;
  }
}
