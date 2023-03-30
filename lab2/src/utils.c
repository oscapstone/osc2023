#include "utils.h"

int strcmp(char *s1, char *s2)
{
    int i;
    for (i = 0; s1[i] != '\0'; i ++)
    {
        if ( s1[i] != s2[i])
        {
            return s1[i] - s2[i];
        }
    }

    return  s1[i] - s2[i];
}

int strncmp(const char *s1, const char *s2, unsigned int n) {
    unsigned int i;

    for (i = 0; i < n; i++) {
        if (*s1 == '\0' || *s2 == '\0' || *s1 != *s2) {
            return *(unsigned char*)s1 - *(unsigned char*)s2;
        }
        s1++;
        s2++;
    }
    return 0;
}

int strtol(const char *str, char **endptr, int base) {
    int result = 0;
    int negative = 0;

    // Handle negative numbers
    if (*str == '-') {
        negative = 1;
        str++;
    }

    // Handle hex prefix (0x or 0X)
    if (base == 0) {
        if (*str == '0' && (*(str+1) == 'x' || *(str+1) == 'X')) {
            base = 16;
            str += 2;
        } else {
            base = 10;
        }
    }

    // Process digits
    while (*str != '\0') {
        int digit;

        if (*str >= '0' && *str <= '9') {
            digit = *str - '0';
        } else if (*str >= 'a' && *str <= 'z') {
            digit = *str - 'a' + 10;
        } else if (*str >= 'A' && *str <= 'Z') {
            digit = *str - 'A' + 10;
        } else {
            break;
        }

        if (digit >= base) {
            break;
        }

        result = result * base + digit;
        str++;
    }

    // Set the end pointer
    if (endptr != 0) {
        *endptr = (char *) str;
    }

    // Handle negative numbers
    if (negative) {
        result = -result;
    }

    return result;
}

void *memcpy(void *dest, const void *src, unsigned int n) {
    char *dst_ptr = (char *) dest;
    const char *src_ptr = (const char *) src;
    unsigned int i;
    for (i = 0; i < n; i++) {
        dst_ptr[i] = src_ptr[i];
    }
    return dest;
}

unsigned int strlen(const char *s) {
    unsigned int size = 0;

    while (*s != '\0') {
        size++; 
        s++;
    }

    return size;
}

unsigned int htonl(unsigned int num){
    return (
        ((num >> 24) & 0x000000FF) | ((num >>  8) & 0x0000FF00) | 
        ((num <<  8) & 0x00FF0000) | ((num << 24) & 0xFF000000)
    );
}
