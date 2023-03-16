#include "stdint.h"

int strcmp(char *s1, char *s2){
    while(*s1!='\0' && *s2!='\0' && *s1==*s2){
        s1 += 1;
        s2 += 1;
    }
    return *s1 - *s2;
}

char* itoa(int64_t val, int base){
    static char buf[32] = {0};
    int i = 30;
    if (val == 0) {
        buf[i] = '0';
        return &buf[i];
    }

    for (; val && i; --i, val /= base)
        buf[i] = "0123456789abcdef"[val % base];

    return &buf[i + 1];
}

int atoi(const char *s, unsigned int size) {
    int num = 0;

    for (unsigned int i = 0; i < size && s[i] != '\0'; i++) {
        if ('0' <= s[i] && s[i] <= '9') {
            num += s[i] - '0';
        } else if ('A' <= s[i] && s[i] <= 'F') {
            num += s[i] - 'A' + 10;
        } else if ('a' <= s[i] && s[i] <= 'f') {
            num += s[i] - 'a' + 10;
        }
    }

    return num;
}

int strncmp(const char *l, const char *r, unsigned int size) {
    for (unsigned int i = 0; i < size; i++) {
        if (l[i] == '\0') {
            return 0;
        }
        if (l[i] != r[i]) {
            return (const char) l[i] - (const char) r[i];
        }
    }

    return 0;
}


unsigned int strlen(const char *s) {
    unsigned int len = 0;

    while (*s != '\0') {
        len++; s++;
    }

    return len;
}