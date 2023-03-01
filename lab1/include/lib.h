#ifndef __LIB_H__
#define __LIB_H__

#include <stdarg.h>
#include <stdint.h>

#define INTEGER_BUF_SIZE 12

int32_t my_strcmp(const char *, const char *);
int32_t my_strncmp(const char *, const char *, uint32_t);
uint32_t my_strlen(const char *);

void *my_memset(void *, int, int);

int dec_to_str(char *, int, int, char);

int my_vsprintf(char *, const char *, va_list);
int my_sprintf(char *, const char *, ...);

#endif
