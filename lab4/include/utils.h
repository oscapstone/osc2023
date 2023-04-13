#ifndef _UTILS_H
#define _UTILS_H

int oct2bin(char *s, int n);
int hex2bin(char *s, int n);
void uart_int(int i);
int atoi(char * c);
void* simple_malloc(void **now, int size);
void buf_clear(char *buf, int BUF_SIZE);
int memcmp(void *s1, void *s2, int n); 
void delay(int sec);

#endif
