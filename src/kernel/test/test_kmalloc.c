#include "test/test_kmalloc.h"
#include "peripherals/mini_uart.h"
#include "mem/mem.h"
#include "random.h"

void test_kmalloc(void *ptr) {
    void *ptrs[1000];
    int idx = 0;
    uart_send_string("Testing large chunk\r\n");
    srand(109550062);
    for(int i = 0, j = 0; i < 40; i ++) {
        uint32_t r = rand();
        if(r & 1) {
            r = rand();
            r %= (1 << 15);
            if(r & 1) {
                ptrs[j ++] = kmalloc(r);
            }
        } else {
            if(j == 0) {
                uart_send_dec(i + 1);
                uart_send_string("\r\n");
                i -= 1;
                continue;
            }
            kfree(ptrs[--j]);
        }
    }
    uart_send_string("test done!\r\n");
}