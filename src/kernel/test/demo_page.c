#include "test/demo_page.h"
#include "random.h"
#include "mem/page.h"
#include "peripherals/mini_uart.h"

void demo_page() {
    void *ptr[10];
    int mb_5 = (1024 * 64);
    for(int i = 0, j = 0; i < 10; i ++) {
            int r = 10000;
            uart_send_string("====allocate page====\r\n");
            uart_send_string("=page size: ");
            uart_send_dec((uint64_t)r);
            uart_send_string(" =\r\n");
            uart_send_string("=====================\r\n");
            ptr[j++] = page_alloc(r);
    }
    int j = 10;

    while(j) {
        uart_send_string("======free page======\r\n");
        uart_send_string("=page addr: ");
        uart_send_u64((uint64_t)ptr[--j]);
        uart_send_string(" =\r\n");
        uart_send_string("=====================\r\n");
        page_free(ptr[j]);
    }
}