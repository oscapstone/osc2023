#include "header/shell.h"
#include "header/uart.h"
#include "header/utils.h"
#include "header/mailbox.h"
#include "header/reboot.h"

void shell() {
    char array_space[256]; // Buffer for user input
    char* input_string = array_space; // Pointer to the start of the input buffer

    while (1) {
        uart_send_string("# "); // Display the shell prompt

        // Initialize input buffer to zero
        for (int i = 0; i < (int)sizeof(array_space); i++) {
            array_space[i] = 0;
        }

        char c;
        // Pointer for the current position being written to in the buffer
        char* input_ptr = input_string;

        // Read characters until a newline is received
        while (1) {
            c = uart_get_char(); // Read a character from UART

            if (c == '\n') {
                *input_ptr = '\0'; // Null-terminate the input string
                uart_send_string("\r\n");
                break; // Exit the loop to process the command
            } else if ((c == '\b' || c == 0x7F) && input_ptr > input_string) {
                // Handle backspace, only if cursor is not at the beginning
                input_ptr--; // Move cursor back
                uart_send_string("\b \b"); // Reflect backspace on terminal
            } else if (c >= 32 && c < 127 && (int)(input_ptr - input_string) < (int)sizeof(array_space) - 1) {
                // If the character is printable and there is space in the buffer, echo it and store it
                uart_send_char(c);
                *input_ptr++ = c;
            }
        }

        // Command processing
        if (string_compare(input_string, "help") == 0) {
            uart_send_string("help   : print this help menu\r\n");
            uart_send_string("hello  : print Hello World!\r\n");
            uart_send_string("info   : get the hardware's information\r\n");
            uart_send_string("reboot : reboot the device\r\n");
        } else if (string_compare(input_string, "hello") == 0) {
            uart_send_string("Hello World!\r\n");
        } else if (string_compare(input_string, "info") == 0) {
            if (get_board_revision() != 0) {
                uart_send_string("Board revision: ");
                uart_binary_to_hex(mailbox[5]);
                uart_send_string("\r\n");
            } else {
                uart_send_string("Unable to query board revision.\r\n");
            }
            if (get_arm_mem() != 0) {
                uart_send_string("ARM memory base address: ");
                uart_binary_to_hex(mailbox[5]);
                uart_send_string("\r\n");
                uart_send_string("ARM memory size: ");
                uart_binary_to_hex(mailbox[6]);
                uart_send_string("\r\n");
            } else {
                uart_send_string("Unable to query ARM memory.\r\n");
            }
        } else if (string_compare(input_string, "reboot") == 0) {
            uart_send_string("Rebooting...\r\n");
            reset(1000); // Invoke reboot functionality
        } else {
            uart_send_string("Unknown command: ");
            uart_send_string(input_string);
            uart_send_string("\r\n");
        }
    }
}
