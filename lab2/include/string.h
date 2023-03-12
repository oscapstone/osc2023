#ifndef _STRING_H
#define _STRING_H

#include "type.h"

int streq(const char *, const char *);
int strneq(const char *, const char *, int);
int strstartwith(const char *, const char *);
char *strcpy(char *, const char *);
int strtoi(const char *, int);
uint32_t strtoui(const char *, int, int);

#endif
