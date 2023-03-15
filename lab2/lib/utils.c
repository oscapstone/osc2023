#include "utils.h"
#include "muart.h"

void printhex(int value) {
    char nums[9]; nums[8] = '\0';

    for (int i = 7; i >= 0; i--) {
        unsigned int x = value % 16; value >>= 4;
        switch (x) {
            case 10: nums[i] = 'A';     break;
            case 11: nums[i] = 'B';     break;
            case 12: nums[i] = 'C';     break;
            case 13: nums[i] = 'D';     break;
            case 14: nums[i] = 'E';     break;
            case 15: nums[i] = 'F';     break;
            default: nums[i] = '0' + x; break;
        }
    }

    mini_uart_puts("0x");
    mini_uart_puts(nums);
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

int strcmp(const char *l, const char *r) {
    while (*l && (*l == *r)) {
        l++; r++;
    }

    return (const char) *l - (const char) *r;
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