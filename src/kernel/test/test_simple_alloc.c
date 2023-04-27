#include "test/test_simple_alloc.h"
#include "mem/mem.h"

void test_simple_alloc(void *ptr) {
    ptr = simple_malloc(128);
    uart_send_string("ptr = ");
    uart_send_u64(ptr);
    uart_send_string("\r\n");
    simple_free(ptr);
    ptr = simple_malloc(128);
    uart_send_string("ptr = ");
    uart_send_u64(ptr);
     ptr = simple_malloc(19);
    uart_send_string("\r\n");
    uart_send_string("ptr = ");
    uart_send_u64(ptr);
    uart_send_string("\r\n");
    simple_free(ptr);
    ptr = simple_malloc(24);
    uart_send_string("ptr = ");
    uart_send_u64(ptr);
    uart_send_string("\r\n");  
}