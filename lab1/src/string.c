#include "string.h"

int streq(const char *str1, const char *str2) {
  while (*str1 != '\0' && *str2 != '\0') {
    if (*str1++ != *str2++)
      return -1;
  }
  if (*str1 != *str2)
      return -1;
  return 0;
}

char *strcpy(char *dst, const char *src) {
  char *save = dst;
  while (*src) {
    *dst++ = *src++;
  }
  return save;
}
