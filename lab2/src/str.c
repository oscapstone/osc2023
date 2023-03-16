#include "str.h"

int strcmp(const char *a, const char *b) {
  do {
    if (a == 0 || b == 0)
      return 3;
    if (*a == '\0' || *b == '\0')
      break;
    if (*a > *b)
      return 1;
    if (*b > *a)
      return -1;
    if (*b++ == *a++)
      continue;
  } while (1);
  if (*a == *b)
    return 0;
  if (*a > *b)
    return 1;
  else
    return -1;
}

int strncmp(const char *a, const char *b, int n) {
  while (n-- > 0) {
    if (a == 0 || b == 0)
      return 3;
    if (*a == '\0' || *b == '\0')
      return 3;
    if (*a > *b)
      return 1;
    if (*b > *a)
      return -1;
    if (*b++ == *a++)
      continue;
  }
  return 0;
}

int strlen(const char *s) {
  int len = 0;
  while (*s) {
    len++;
    s++;
  }
  return len;
}

void *memset(void *s, char c, unsigned int n) {
  if (n < 0 || s == 0)
    return s;
  char *r = (char *)s;

  for (int i = 0; i < n; i++) {
    *r = c;
    r++;
  }
  return s;
}
