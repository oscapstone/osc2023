#include "utils.h"
#include "muart.h"

int strcmp(const char *l, const char *r) {
    while (*l && (*l == *r)) {
        l++; r++;
    }

    return (const char) *l - (const char) *r;
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

    mini_uart_puts("0x");
    mini_uart_puts(nums);
}