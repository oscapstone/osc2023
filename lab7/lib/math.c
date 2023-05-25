#include "math.h"

int log(int n, int base) {
    int x = 1;
    int ret = 0;
    while (x <= n) {
        x *= base;
        ret++;
    }
    return ret;
}

int pow(int base, int pow) {
    int ret = 1;
    for (int i=0; i<pow; i++) {
        ret *= base;
    }
    return ret;
}