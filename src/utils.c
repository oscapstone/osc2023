#include "utils.h"
#include "stdint.h"
#pragma GCC push_options
#pragma GCC optimize ("unroll-loops")
void delay(unsigned int t) {
    for(; t > 0; t--) {
        asm ("nop");
    }
}

void *memset(void *s, int c, size_t n) {
    unsigned char *p = s;
    while (n-- > 0) {
        *p++ = (unsigned char)c;
    }
    return s;
}

void *memcpy(void *dest, const void *src, unsigned int len)
{
  char *d = dest;
  const char *s = src;
  // while (len > 8) {
  //   *(uint64_t *)d = *(uint64_t *)s;
  //   d += 8;
  //   s += 8;
  //   len -= 8;
  // }
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

char* strncpy(char* dest, const char* src, size_t n) {
    char* result = dest;
    while (*src != '\0' && n--) {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
    return result;
}

char* strcpy(char* dest, const char* src) {
    char* result = dest;
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
    return result;
}


char* strpbrk(const char* str, const char* delimiters) {
  const char* currentChar;
  const char* currentDelimiter;

  for (currentChar = str; *currentChar != '\0'; currentChar++) {
    for (currentDelimiter = delimiters; *currentDelimiter != '\0'; currentDelimiter++) {
      if (*currentChar == *currentDelimiter) {
        return (char*) currentChar;
      }
    }
  }

  return NULL;
}

static int in_str(char c, const char *str)
{
  for (char *tmp = str; *tmp; tmp++) {
    if (c == *tmp) return 1;
  }
  return 0;
}

unsigned long long strspn(const char *str1, const char *str2)
{
  const char *org = str1;
  for (; *str1 && in_str(*str1, str2); str1++);
  return str1 - org;
}

unsigned long long strcspn(const char *str1, const char *str2)
{
  const char *org = str1;
  for (; *str1 && !in_str(*str1, str2); str1++);
  return str1 - org;
}

char *__strtok_r (char *s, const char *delim, char **save_ptr)
{
  char *end;

  if (s == NULL)
    s = *save_ptr;

  if (*s == '\0')
    {
      *save_ptr = s;
      return NULL;
    }

  /* Scan leading delimiters.  */
  s += strspn (s, delim);
  if (*s == '\0')
    {
      *save_ptr = s;
      return NULL;
    }

  /* Find the end of the token.  */
  end = s + strcspn (s, delim);
  if (*end == '\0')
    {
      *save_ptr = end;
      return s;
    }

  /* Terminate the token and make *SAVE_PTR point past it.  */
  *end = '\0';
  *save_ptr = end + 1;
  return s;
}

char *strtok(char *s, const char *delim)
{
  static char *prev;
  return __strtok_r(s, delim, &prev);
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

static unsigned char2unsigned(char c, unsigned invalid_res)
{
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
      return c - '0';
      break;
    case 'a': case 'A': 
    case 'b': case 'B': 
    case 'c': case 'C': 
    case 'd': case 'D': 
    case 'e': case 'E': 
    case 'f': case 'F': 
    //將字元轉成小寫: 免除使用分支
      return (c  | ' ') - 'a' + 10;
      break;
    default: return invalid_res;  // Invalid character, return 0
  }
}

unsigned hex2unsigned(char *s) {
    unsigned res = 0;
    for (int i = 0; i < 8 && s[i] != '\0'; i++) {
      res = (res << 4) | char2unsigned(s[i], 0);
    }
    return res;
}

unsigned long long hex2ull(char *s) {
    unsigned long long res = 0;
    for (int i = 0; i < 16 && s[i] != '\0'; i++) {
      res = (res << 4) | char2unsigned(s[i], 0);
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

unsigned long long alignToNextPowerOf2(unsigned long long num) {
    num--;
    num |= num >> 1;
    num |= num >> 2;
    num |= num >> 4;
    num |= num >> 8;
    num |= num >> 16;
    num |= num >> 32;
    num++;
    return num;
}

unsigned ul_log2(unsigned long long n) {
    unsigned result = 0;
    while (n >>= 1) {
        result++;
    }
    return result;
}