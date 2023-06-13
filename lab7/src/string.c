#include "string.h"
#include <stddef.h>
#include "uart.h"

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

int strncmp (const char *s1, const char *s2, unsigned long long n)
{
  unsigned char c1 = '\0';
  unsigned char c2 = '\0';
  if (n >= 4)
    {
      size_t n4 = n >> 2;
      do
        {
          c1 = (unsigned char) *s1++;
          c2 = (unsigned char) *s2++;
          if (c1 == '\0' || c1 != c2)
            return c1 - c2;
          c1 = (unsigned char) *s1++;
          c2 = (unsigned char) *s2++;
          if (c1 == '\0' || c1 != c2)
            return c1 - c2;
          c1 = (unsigned char) *s1++;
          c2 = (unsigned char) *s2++;
          if (c1 == '\0' || c1 != c2)
            return c1 - c2;
          c1 = (unsigned char) *s1++;
          c2 = (unsigned char) *s2++;
          if (c1 == '\0' || c1 != c2)
            return c1 - c2;
        } while (--n4 > 0);
      n &= 3;
    }
  while (n > 0)
    {
      c1 = (unsigned char) *s1++;
      c2 = (unsigned char) *s2++;
      if (c1 == '\0' || c1 != c2)
        return c1 - c2;
      n--;
    }
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
  {
    *d++ = *s++;
  }
  return dest;
}

char* strchr (register const char *s, int c)
{
  do {
    if (*s == c)
      {
        return (char*)s;
      }
  } while (*s++);
  return (0);
}

// A simple atoi() function
int atoi(char* str)
{
  // Initialize result
  int res = 0;

  // Iterate through all characters
  // of input string and update result
  // take ASCII character of corresponding digit and
  // subtract the code from '0' to get numerical
  // value and multiply res by 10 to shuffle
  // digits left to update running total
  for (int i = 0; str[i] != '\0'; ++i)
  {
    if(str[i] > '9' || str[i] < '0')return res;
    res = res * 10 + str[i] - '0';
  }

  // return result.
  return res;
}

void *memset(void *s, int c, size_t n)
{
  char *start = s;
  for (size_t i = 0; i < n; i++)
  {
    start[i] = c;
  }

  return s;
}