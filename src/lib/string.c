#include <string.h>

int strcmp(const char *X,const char *Y){
    while(*X){
        if(*X != *Y)break;
        X++;
        Y++;
    }
    return *(const unsigned char*)X - *(const unsigned char*)Y;
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