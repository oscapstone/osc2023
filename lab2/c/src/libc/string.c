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
