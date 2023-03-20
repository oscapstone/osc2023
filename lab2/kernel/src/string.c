#include "string.h"

int strncmp(const char *s1, const char *s2, int n) {
  while (n-- > 0) {
    if (s1 == 0 || s2 == 0)
      return 1;
    if (*s1 == '\0' || *s2 == '\0')
      return 1;
    if (*s1 > *s2 || *s1 < *s2)
      return 1;
    if (*s1++ == *s2++)
      continue;
  }
  return 0;
}

void* memset(char *buffer, int size)
{
    for (int i = 0; i < size; i++)
    {
            buffer[i] = '\0';
    }
}

unsigned int strlen(const char *s) {
    unsigned int len = 0;

    while (*s != '\0') {
        len++; s++;
    }

    return len;
}
