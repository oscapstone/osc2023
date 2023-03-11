/*
 * The file contain string operations, such
 * as strcmp, strlen, memset, etc.
 *
 * */
#ifndef STR_H
#define STR_H

int strcmp(const char *, const char *);
int strncmp(const char *, const char *, int n);
int strlen(const char *);
void *memset(void *, char, unsigned int);

#endif // STR_H
