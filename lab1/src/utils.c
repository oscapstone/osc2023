#include "utils.h"
#include "uart.h"

void delay(int waits) {
    while (waits--);
}

int str_len(char *input) {
    int count = 0;
    while (*input != '\0') {
        count++;
        input++;
    }
    return 0;
}

int str_cmp(char *input1, char* input2) {
	while (*input1 && *input2 && *input1 == *input2) {
        input1++;
        input2++;
    }
    return *input1 - *input2;
}

void print_hex(int dec) {
    char hex[9]; 
    hex[8] = '\0';
    unsigned int digit;

    for (int i = 7; i >= 0; i--) {
        digit = dec % 16; 
        dec /= 16;

        switch (digit) {
            case 10: hex[i] = 'A';         break;
            case 11: hex[i] = 'B';         break;
            case 12: hex[i] = 'C';         break;
            case 13: hex[i] = 'D';         break;
            case 14: hex[i] = 'E';         break;
            case 15: hex[i] = 'F';         break;
            default: hex[i] = '0' + digit; break;
        }
    }

    uart_writes("0x");
    uart_writes(hex);
}

