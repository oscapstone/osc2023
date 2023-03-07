#include "uart.h"
#include "shell.h"
#include "hardware_info.h"

int main(void)
{
    uart_init();
    uart_flush();
    /* print hardware information */
    unsigned int request_message[8] = {0};
    uart_write_string("BOARD REVISION: ");
    get_hw_info(GET_BOARD_REVISION, &request_message);
    uart_write_string("ARM MEMORY: ");
    get_hw_info(GET_ARM_MEMORY, &request_message);
    uart_write_string("MAC ADDRESS: ");
    get_hw_info(GET_MAC_ADDRESS, &request_message);
    uart_write_string("TEMPERATURE: ");
    get_hw_info(GET_TEMPERATURE, &request_message);
    
    /* shell */
    unsigned int read_cnt;
    char input_buffer[MAX_SHELL_INPUT];

    while (1) {
        read_cnt = shell_read_string(input_buffer, MAX_SHELL_INPUT);
        shell_process_cmd(input_buffer, read_cnt);
    }

    return 0;
}
