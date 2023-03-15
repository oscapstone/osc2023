#include "string.h"

char strcmp(char * s, char * t) {
    while (*s && *t && *s == *t) {
        ++s; ++t;
    }
    return *s - *t;
}
