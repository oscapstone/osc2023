#include "type.h"
#ifndef _UTILS_H
#define _UTILS_H


extern void delay(unsigned long long);
extern void write_reg_32(unsigned long, unsigned int);
extern unsigned int read_reg_32(unsigned long);
void memcpy(void* dest, void *src, unsigned long long size);
void memset(void *dest, char val, unsigned int size);
int strncmp(const char *s1, const char *s2, unsigned int maxlen);
int strlen(const void *buf);
uint64_t ntohl(uint64_t);
uint32_t ntohi(uint32_t);
uint16_t ntohs(uint16_t);

#endif