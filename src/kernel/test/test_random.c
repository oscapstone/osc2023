#include "test/test_random.h"
#include "peripherals/mini_uart.h"
#include "random.h"

void test_random() {
    srand(109550062);
    for(int i = 0; i < 10; i ++) {
        uint32_t r = rand();
        uart_send_u32(r);
        uart_send_string("\r\n");
    }
}