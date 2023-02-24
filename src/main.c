#include "uart.h"
#include "shell.h"
#include "hardware_info.h"

int main(void)
{
    uart_init();
    uart_flush();
    /* print hardware information */
    uart_write_string("BOARD REVISION: ");
    get_hw_info(GET_BOARD_REVISION);
    uart_write_string("ARM MEMORY: ");
    get_hw_info(GET_ARM_MEMORY);
    uart_write_string("MAC ADDRESS: ");
    get_hw_info(GET_MAC_ADDRESS);
    //uart_write_string("CLOCKS: ");
    //get_hw_info(GET_CLOCKS);
    //uart_write_string("MAX CLOCK RATE: ");
    //get_hw_info(GET_MAX_CLOCK_RATE);
    uart_write_string("TEMPERATURE: ");
    get_hw_info(GET_TEMPERATURE);
    
    /* shell */
    unsigned int read_cnt;
    char input_buffer[MAX_SHELL_INPUT];

    while (1) {
        read_cnt = shell_read_string(input_buffer, MAX_SHELL_INPUT);
        shell_process_cmd(input_buffer, read_cnt);
    }

    return 0;
}
