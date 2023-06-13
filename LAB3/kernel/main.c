// #include "mini_uart.h"
// #include "dtb.h"
// #include "exception_c.h"
// #include "utils_s.h"
// #include "shell.h"
// #include "timer.h"

// extern void *_dtb_ptr;

// void kernel_main(void)
// {
//     // uart_init();
//     timeout_event_init();
//     uart_send_string("Hello, world!\n");
//     fdt_traverse(get_initramfs_addr, _dtb_ptr);
//     int el = get_el();
//     uart_printf("kernel Exception level: %d\n",el);
//     enable_interrupt();
//     shell();
// }

// Include necessary header files
#include "mini_uart.h"
#include "dtb.h"
#include "exception_c.h"
#include "utils_s.h"
#include "shell.h"
#include "timer.h"

// Declare external variable
extern void *_dtb_ptr;

// Main function
void kernel_main(void)
{
    uart_init();
    // Initialize timeout event
    timeout_event_init();

    // Send greeting message
    uart_send_string("Hello, world!\n");

    // Traverse device tree blob
    fdt_traverse(get_initramfs_addr, _dtb_ptr);

    // Get exception level
    int el = get_el();

    // Print exception level
    uart_printf("kernel Exception level: %d\n",el);

    // Enable interrupt
    enable_interrupt();

    // Start shell
    shell();
}
