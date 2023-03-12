#include <string.h>

int strcmp(const char* X,const char* Y){
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