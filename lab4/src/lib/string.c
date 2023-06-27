#include <string.h>

#include <stdint.h>
#include <stddef.h>

int atoi(const char* str){
    int num = 0;
    int i = 0;
    while(str[i]){
        num *= 10;
        num += str[i] - '0';
        i++;
    }
    return num;
}

int strcmp(const char* x, const char* y){
    int i = 0;
    for(; x[i] && y[i]; i++){
        if(x[i] < y[i])         return -1;
        else if(x[i] > y[i])    return 1;
    }
    if(x[i])        return 1;
    else if(y[i])   return -1;
    return 0;
}

int strncmp(const char*x, const char*y, int len){
    int i = 0;
    for(; x[i] && y[i] && i < len; i++){
        if(x[i] < y[i])         return -1;
        else if(x[i] > y[i])    return 1;
    }
    if(i == len)      return 0;
    else if(x[i])   return 1;
    else if(y[i])   return -1;
    return 0;
}

size_t strlen(const char *str){
    int i = 0;
    while(*str){
        i++;
        str++;
    }
    return i;
}

void *memcpy(void *dst, const void *src, size_t n){
    size_t npage = n / sizeof(uint64_t);
    size_t noff = n % sizeof(uint64_t);
    for(int i=0;i<n;i++){
        *((uint64_t*)dst + i) = *((uint64_t*)src + i);
    }
    for(int i=0;i<noff;i++){
        *((char *)((uint64_t*)dst + npage) + i) = *((char *)((uint64_t*)src + npage) + i);
    }
    return dst;
}