#include "utils.h"
#pragma GCC push_options
#pragma GCC optimize ("unroll-loops")
void delay(unsigned int t) {
    for(; t > 0; t--) {
        asm ("nop");
    }
}

void *memcpy (void *dest, const void *src, unsigned int len)
{
  char *d = dest;
  const char *s = src;
  while (len--)
    *d++ = *s++;
  return dest;
}
#pragma GCC pop_options

static int in_delm(char c, const char *delm)
{
  const int delm_len = strlen(delm);
  for (int i = 0; i < delm_len; i++) {
    if (c == delm[i]) {
      return 1;
    }
  }
  return 0;
}

char *my_strtok(char *str, const char *delm)
{
  static char *ptr = NULL;
  if (str != NULL)
    ptr = str;
  while (ptr != NULL) {
    //check if *ptr is in delm
    //if yes, pass the ptr
    if (in_delm(*ptr, delm)) {
      ptr++;
    } else {
      //the first position that is not in delm
      //update ptr for next call
      char *next_ptr = ptr;
      while (*next_ptr && !in_delm(*next_ptr, delm))
        next_ptr++;
      char *ret = ptr;
      ptr = next_ptr;
      return ret;
    }
  }
  return ptr;
}

int strcmp(char *s1, char *s2) 
{
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strncmp(char *s1, char *s2, unsigned n)
{
    for (unsigned i = 0; i < n; i++) {
        if (s1[i] != s2[i]) {
            return (s1[i] < s2[i]) ? -1 : 1;
        }
        if (s1[i] == '\0') {
            return 0;
        }
    }
    return 0;
}

unsigned strlen(char *s)
{
  char *end = s;
  while (*end) end++;
  return end - s;
}

unsigned hex2unsigned(char *s) {
    unsigned res = 0;
    char c;
    for (int i = 0; i < 8 && s[i] != '\0'; i++) {
      c = s[i];
      unsigned val = 0;
      switch (c) {
          case '0': 
          case '1': 
          case '2': 
          case '3': 
          case '4': 
          case '5': 
          case '6': 
          case '7': 
          case '8': 
          case '9': 
            val = c - '0';
            break;
          case 'a': case 'A': 
          case 'b': case 'B': 
          case 'c': case 'C': 
          case 'd': case 'D': 
          case 'e': case 'E': 
          case 'f': case 'F': 
          //將字元轉成小寫: 免除使用分支
            val = (c  | ' ') - 'a' + 10;
            break;
          default: return 0;  // Invalid character, return 0
      }
      res = (res << 4) | val;
    }
    return res;
}

unsigned long long hex2ull(char *s) {
    unsigned long long res = 0;
    char c;
    for (int i = 0; i < 16 && s[i] != '\0'; i++) {
      c = s[i];
      unsigned long long val = 0;
      switch (c) {
          case '0': 
          case '1': 
          case '2': 
          case '3': 
          case '4': 
          case '5': 
          case '6': 
          case '7': 
          case '8': 
          case '9': 
            val = c - '0';
            break;
          case 'a': case 'A': 
          case 'b': case 'B': 
          case 'c': case 'C': 
          case 'd': case 'D': 
          case 'e': case 'E': 
          case 'f': case 'F': 
          //將字元轉成小寫: 免除使用分支
            val = (c  | ' ') - 'a' + 10;
            break;
          default: return 0;  // Invalid character, return 0
      }
      res = (res << 4) | val;
    }
    return res;
}

int strstartswith(char *str, char *prefix)
{
  const int pref_len = strlen(prefix);
  const int str_len = strlen(str);
  if (str_len < pref_len) return 0;
  return (strncmp(str, prefix, pref_len) == 0);
}

#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
int atoi(const char *str) {
    int result = 0;
    int sign = 1;
    int i = 0;

    // Skip whitespace
    while (str[i] == ' ') {
        i++;
    }

    // Check for sign
    if (str[i] == '-') {
        sign = -1;
        i++;
    } else if (str[i] == '+') {
        i++;
    }

    // Parse digits
    while (IS_DIGIT(str[i])) {
        result = result * 10 + (str[i] - '0');
        i++;
    }

    return sign * result;
}