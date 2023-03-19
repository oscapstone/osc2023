#include "string.h"

char strcmp(char * s, char * t) {
    while (*s && *t && *s == *t) {
        ++s; ++t;
    }
    return *s - *t;
}

unsigned int strlen(char * s) {
    unsigned int len = 0;
    while (*(s++) != '\0') {
        ++len;
    }
    return len;
}
