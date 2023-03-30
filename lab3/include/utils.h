#ifndef __UTILS_H__
#define __UTILS_H__

#define BUFSIZE 256

void printdec(unsigned long value);
void printhex(unsigned long value);
int  atoi(const char *s, unsigned int size);
int  strcmp(const char *l, const char *r);
int  strncmp(const char *l, const char *r, unsigned int size);
unsigned int strlen(const char *s);

#endif