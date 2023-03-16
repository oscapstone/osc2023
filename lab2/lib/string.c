#include "string.h"

inline int isdigit(char ch) {
  return (ch >= '0') & (ch <= '9');
}

inline int isupper(char ch) {
  return (ch >= 'A') & (ch <= 'Z');
}

int streq(const char *str1, const char *str2) {
  while (*str1 != '\0' && *str2 != '\0') {
    if (*str1++ != *str2++)
      return -1;
  }
  if (*str1 != *str2)
      return -1;
  return 0;
}

int strneq(const char *str1, const char *str2, int len) {
  int i;
  for (i = 0; i < len; i++) {
    if (str1[i] != str2[i]) return -1;
  }
  return 0;
}

int strstartwith(const char *string, const char *sub) {
  while (*sub) {
    if (*string == '\0')
      return -1;
    if (*string++ != *sub++)
      return -1;
  }
  return 0;
}

char *strcpy(char *dst, const char *src) {
  char *save = dst;
  while (*src) {
    *dst++ = *src++;
  }
  return save;
}

int strtoi(const char *str, int base) {
  int num = 0;
  int neg = 0;
  if (*str == '-') {
    str++;
    neg = 1;
  }

  if (base == 16) {
    if (*str == '0' && (*(str+1) == 'x' || *(str+1) == 'X')) {
      str += 2;
    }
    while (*str) {
      num <<= 4;
      if (isdigit(*str))
        num += *str - '0';
      else
        num += isupper(*str) ? (*str - 'A' + 10) : (*str - 'a' + 10);
      str++;
    }
  } else if (base == 10) {
    while (*str) {
      num *= 10;
      if (isdigit(*str))
        num += *str - '0';
      else
        num += isupper(*str) ? (*str - 'A' + 10) : (*str - 'a' + 10);
      str++;
    }
  }
  if (neg)
    num = -num;
  return num;
}

uint32_t strtoui(const char *str, int len, int base) {
  uint32_t num = 0;

  if (base == 16) {
    if (*str == '0' && (*(str+1) == 'x' || *(str+1) == 'X')) {
      str += 2;
    }
    for (int idx = 0; idx < len && str[idx]; idx++) {
      num <<= 4;
      if (isdigit(str[idx]))
        num += str[idx] - '0';
      else
        num += isupper(str[idx]) ? (str[idx] - 'A' + 10) : (str[idx] - 'a' + 10);
    }
  } else if (base == 10) {
    for (int idx = 0; idx < len && str[idx]; idx++) {
      num *= 10;
      if (isdigit(str[idx]))
        num += str[idx] - '0';
      else
        num += isupper(str[idx]) ? (str[idx] - 'A' + 10) : (str[idx] - 'a' + 10);
    }
  }
  return num;
}

int strlen(const char *str) {
  int len = 0;
  while (*str) {
    str++;
    len++;
  }
  return len;
}
