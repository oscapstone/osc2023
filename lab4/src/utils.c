#include"mini_uart.h"

/**
 * Helper function to convert ASCII octal number into binary
 * s string
 * n number of digits
 */
int oct2bin(char *s, int n)
{
    int r=0;
    while(n-->0) {
        r<<=3;
        r+=*s++-'0';
    }
    return r;
}

int hex2bin(char *s, int n){
    int r=0;
    while(n-->0) {
        r<<=4;
        if (*s<='F' && *s>='A'){
            r+=*s++-0x37;
        }
        else r+=*s++-'0';
    }
    return r;
}

int exp(int i, int j){
    int ret = 1;
    for (; j>0; j--){
        ret*=i;
    }
    return ret;
}

void uart_int(int i){
    int e = 0;
    int temp = i;
    while ((i /= 10) >0){
        e++;
    }
    for(;e; e--){
        uart_send((temp/exp(10, e))%10+'0');
    }
    uart_send('0'+(temp%10));
}

int atoi(char * c){
    int ret = 0;
    for(int i=sizeof(c)-1; i>=0; i--){
        ret+= (c[i]-0x48);
    }
    return ret;
}

// void uart_printf(char *s, )

void* simple_malloc(void **now, int size) {
    void *ret = *now;
    *now = *(char **)now + size;
    return ret;
}


void buf_clear(char *buf, int BUF_SIZE){
    for(int i = 0; i < BUF_SIZE; i++){
        buf[i] = '\0';
    }
}

int memcmp(void *s1, void *s2, int n)
{
    unsigned char *a=s1,*b=s2;
    while(n-->0){ if(*a!=*b) { return *a-*b; } a++; b++; }
    return 0;
}

void delay(int sec){
	for (int i = 0; i < sec*1000000000 ; i++) {
		asm("nop;");
	}	
}
