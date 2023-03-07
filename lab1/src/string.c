#include "string.h"
#include <stddef.h>

int strcmp (const char *p1, const char *p2)
{
  const unsigned char *s1 = (const unsigned char *) p1;
  const unsigned char *s2 = (const unsigned char *) p2;
  unsigned char c1, c2;
  do
    {
      c1 = (unsigned char) *s1++;
      c2 = (unsigned char) *s2++;
      if (c1 == '\0')
        return c1 - c2;
    }
  while (c1 == c2);
  return c1 - c2;
}

char* strcat (char *dest, const char *src)
{
  strcpy (dest + strlen (dest), src);
  return dest;
}

char* strcpy (char *dest, const char *src)
{
  return memcpy (dest, src, strlen (src) + 1);
}

unsigned long long strlen(const char *str)
{
  size_t count = 0;
  while((unsigned char)*str++)count++;
  return count;
}

char* memcpy (void *dest, const void *src, unsigned long long len)
{
  char *d = dest;
  const char *s = src;
  while (len--)
    *d++ = *s++;
  return dest;
}