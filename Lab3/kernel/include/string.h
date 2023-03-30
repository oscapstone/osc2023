#ifndef STRING_H
#define STRING_H
#include "type.h"


#define VSPRINT_MAX_BUF_SIZE 0x1000

unsigned int sprintf(char *dst, char* fmt, ...);
unsigned int vsprintf(char *dst,char* fmt, __builtin_va_list args);

int  strcmp     ( char * s1, char * s2 );
void strset     ( char * s1, int c, int size );
int  strlen     ( char * s1 );
void itoa       ( int x, char str[], int d);
void ftoa       ( float n, char* res, int afterpoint ); 
void reverse    ( char *s1 );
void itohex_str ( uint64_t d, int size, char * s1 );
int  strncmp     (const char *s1, const char *s2, unsigned long long n);
unsigned long hextoint(char* addr, const int size);


char* memcpy(void *dest, const void *src, unsigned long long len);
char* strcpy (char *dest, char *src);
char* str_SepbySpace(char* head);
int   atoi(char* s1);


#endif