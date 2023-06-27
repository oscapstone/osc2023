#ifndef __UTILS__
#define __UTILS__
#include <stddef.h>
#include <stdint.h>
#define memory_read(addr) *(uint32_t *)(addr)
#define memory_write(addr,val) *(uint32_t *)(addr) = (uint32_t)(val)
#define align(tmp) (void*)((tmp >> 2) + 1 << 2)
void wait_cycles(uint32_t times);
uint32_t hex2u32_8(char *buf);
uint16_t ntohs(uint16_t tmp);
uint32_t ntohl(uint32_t tmp);
uint64_t ntohll(uint64_t tmp);
uint16_t htons(uint16_t tmp);
uint32_t htonl(uint32_t tmp);
uint64_t htonll(uint64_t tmp);
#endif