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

int strcmp(char *a, char *b)
{
    char *p1 = a, *p2 = b;
    while (*p1 != '\0' && *p2 != '\0' && *p1 == *p2) {
        p1++;
        p2++;
    }
    return (*p1 != '\0' || *p2 != '\0');
}