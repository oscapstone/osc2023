#include "utils.h"

int strcmp(const char *l, const char *r) {
    while (*l && (*l == *r)) {
        l++; r++;
    }

    return (const char) *l - (const char) *r;
}