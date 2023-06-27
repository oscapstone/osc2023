#ifndef _STRING_H
#define _STRING_H

int strcmp(const char *X, const char *Y);
int strcasecmp(const char *X, const char *Y);
int strncmp(const char *X, const char *Y, int n);
int strlen(const char *str);
int strcpy(char *dst, const char *src);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, int n);
int atoi(const char *str);

#endif  /* _STRING_H */