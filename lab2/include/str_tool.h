#ifndef _STR_TOOL_H
#define _STR_TOOL_H
#include "stdint.h"

#define BUFSIZE 64

int strcmp(char*, char*);
char* itoa(int64_t, int);
int  strncmp(const char *l, const char *r, unsigned int size);
int  atoi(const char *s, unsigned int size);
unsigned int strlen(const char *s) ;
#endif