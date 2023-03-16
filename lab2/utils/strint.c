#include "mini_uart.h"

typedef unsigned long long int uint64_t;
typedef unsigned char uint8_t;

uint8_t hex_to_int8(char hex){
    if(hex >= '0' && hex <= '9')
        return hex-'0';
    else if(hex >= 'A' && hex <= 'Z')
        return hex-'A'+10;
    else if(hex >= 'a' && hex <= 'z')
        return hex-'a'+10;
    else
        return -1;
}

uint64_t hex_to_int64(char* num){
    uint64_t res=0;
    for(int i=0; i<8; i++){
        res = (res<<4) + hex_to_int8(num[i]);
    }
    return res;
}


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

    uart_puts("0x");
    uart_puts(nums);
}