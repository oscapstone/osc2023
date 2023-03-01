#include "lib.h"

int my_strcmp(const char* lhs, const char* rhs) {
  while (*lhs == *rhs) {
    if (*lhs == 0) {
      return 0;
    }
    lhs++;
    rhs++;
  }
  return *(unsigned char*)lhs - *(unsigned char*)rhs;
}

int my_strncmp(const char* lhs, const char* rhs, uint32_t n) {
  unsigned char c1;
  unsigned char c2;
  while (n--) {
    c1 = (unsigned char)*lhs++;
    c2 = (unsigned char)*rhs++;
    if (c1 != c2) {
      return c1 - c2;
    }
    if (c1 == '\0') {
      return 0;
    }
  }
  return 0;
}

uint32_t my_strlen(const char* str) {
  const char* s;
  for (s = str; *s; s++) {
  }
  return s - str;
}

void* my_memset(void* buf, int val, int len) {
  uint8_t* p = (uint8_t*)buf;
  while (len-- > 0) {
    *p = val;
    p++;
  }
  return buf;
}

int dec_to_str(char* buf, int d, int base, char letter_base) {
  int neg = 0;
  int idx = 0;

  if (d < 0) {
    d = -d;
    neg = 1;
  }

  char v;
  do {
    v = d % base;
    buf[idx] = v < 10 ? v + '0' : letter_base + v - 10;

    d /= base;
    idx++;
  } while (d);

  if (neg) {
    buf[idx] = '-';
    idx++;
  }

  for (int i = (idx - 1) / 2; i >= 0; i--) {
    char t = buf[i];
    buf[i] = buf[idx - i - 1];
    buf[idx - i - 1] = t;
  }

  return idx;
}

// FIXME: negative hex
int my_vsprintf(char* buffer, const char* format, va_list va) {
  char* buffer_orig = buffer;

  while (*format) {
    if (*format != '%') {
      *buffer = *format;
      buffer++;
      format++;
      continue;
    }

    format++;
    switch (*format) {
      case 'd':
      case 'x':
      case 'X': {
        int base = *format == 'd' ? 10 : 16;
        char letter_base = *format == 'x' ? 'a' : 'A';
        int d = va_arg(va, int);

        int len = dec_to_str(buffer, d, base, letter_base);
        buffer += len;
        break;
      }

      case 's': {
        const char* s = va_arg(va, const char*);
        const int len = my_strlen(s);
        for (int i = 0; i < len; i++) {
          *buffer = s[i];
          buffer++;
        }
        break;
      }

      default:
        *buffer = *format;
        buffer++;
    }

    format++;
  }

  return buffer - buffer_orig;
}

int my_sprintf(char* buffer, const char* format, ...) {
  va_list va;
  va_start(va, format);
  int res = my_vsprintf(buffer, format, va);
  va_end(va);
  return res;
}
