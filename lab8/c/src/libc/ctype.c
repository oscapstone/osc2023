#include "oscos/libc/ctype.h"

int isalnum(const int c) { return isalpha(c) || isdigit(c); }

int isalpha(const int c) { return isupper(c) || islower(c); }

int isdigit(const int c) { return '0' <= c && c <= '9'; }

int islower(const int c) { return 'a' <= c && c <= 'z'; }

int isupper(const int c) { return 'A' <= c && c <= 'Z'; }

int toupper(const int c) { return islower(c) ? c | 0x20 : c; }
