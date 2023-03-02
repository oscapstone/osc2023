#ifndef __UTILS__
#define __UTILS__
#include <stdint.h>
#define memory_read(addr) *(uint32_t *)(addr)
#define memory_write(addr,val) *(uint32_t *)(addr) = (uint32_t)(val)
void wait_cycles(uint32_t times);
int strncmp(const char*x, const char*y, int len);
#endif