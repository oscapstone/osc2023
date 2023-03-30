#include "utils.h"

static inline void exchg_char(char * c, char * d) {
    *c ^= *d;
    *d ^= *c;
    *c ^= *d;
}

static inline unsigned int big2little32u(unsigned int n) {
    char * c = (char *)&n;
    exchg_char(c + 0, c + 3);
    exchg_char(c + 1, c + 2);
    return n;
}

unsigned int load_big32u(char * addr) {
    unsigned int big = *(unsigned int *)addr;
    unsigned int little = big2little32u(big);
    return little;
}