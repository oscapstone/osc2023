#include <utils.h>

void wait_cycles(uint32_t times){
    while(times--) asm volatile("nop");
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