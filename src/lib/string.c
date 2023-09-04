#include <string.h>

int strcmp(const char *X,const char *Y){
    while(*X){
        if(*X != *Y)break;
        X++;
        Y++;
    }
    return *(const unsigned char*)X - *(const unsigned char*)Y;
}

int strcasecmp(const char *X, const char *Y)
{
    char c1, c2;

    while (1) {
        c1 = *X++;
        c2 = *Y++;

        if (!c1 || !c2) {
            break;
        }

        if ('A' <= c1 && c1 <= 'Z') {
            c1 |= 0x20;
        }

        if ('A' <= c2 && c2 <= 'Z') {
            c2 |= 0x20;
        }

        if (c1 != c2) {
            break;
        }
    }

    return c1 - c2;
}

int strncmp(const char *X, const char *Y, int n){
    while(n && *X && (*X == *Y)){
        X++;
        Y++;
        n--;
    }
    if(n==0) return 0;
    return *(const unsigned char*)X - *(const unsigned char*)Y;  
}

int strlen(const char *str){
    int ret = 0 ;
    while(*str++)
        ret++;
    return ret;
}

int strcpy(char *dst, const char *src)
{
    int ret = 0;

    while (*src) {
        *dst = *src;
        dst++;
        src++;
        ret++;
    }

    *dst = '\0';

    return ret;
}

char *strcat(char *dest, const char *src){
    char *t;
    t = dest;
    while(*t){
        t++;
    }
    while(*src){
        *t = *src;
        t++;
        src++;
    }
    *t = '\0';
    return dest;
}

char *strncat(char *dest, const char *src, int n){
    char *t;
    t = dest;
    while(*t)
        t++;
    while(n > 0 && *src){
        *t = *src;
        t++;
        src++;
        n--;
    }
    *t = '\0';
    return dest;
}

int atoi(const char *str){
    int i = 0 , j = 0;
    while(*str){
        if('0'>*str || *str>'9')
            return -1;
        i*=10;
        j=i+(*str-'0');

        // integer overflow
        if(j < i)
            return -1;
        i = j;
        str++;
    }
    return i;
}