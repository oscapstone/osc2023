#include "uart.h"
#include "utils.h"
#include "reboot.h"
#include "shell.h"
enum ANSI_ESC {
    Unknown,
    CursorForward,
    CursorBackward,
    Delete
};

enum ANSI_ESC decode_csi_key() {
    char c = uart_read();
    switch (c)
    {
    case 'C':
        return CursorForward;
        break;
    case 'D':
        return CursorBackward;
        break;
    case '3':
        return uart_read() == '~' ? Delete : Unknown;
        break;
    default:
        return Unknown;
        break;
    }
}

enum ANSI_ESC decode_ansi_escape() {
    return uart_read() == '[' ? decode_csi_key() : Unknown;
}

unsigned int shell_read_string(char* cmd, unsigned int cmd_size) {
    uart_write_string("# ");

    int idx = 0, end = 0, i, last_end = 0;
    cmd[0] = '\0';
    char c;
    while ((c = uart_read()) != '\n') {
        // Decode CSI key sequences
        switch (c)
        {
        case 27:
            enum ANSI_ESC key = decode_ansi_escape();
            switch (key) {
                case CursorForward:
                    if (idx < end) idx++;
                    break;

                case CursorBackward:
                    if (idx > 0) idx--;
                    break;

                case Delete:
                    // left shift command
                    for (i = idx; i < end; i++) {
                        cmd[i] = cmd[i + 1];
                    }
                    cmd[--end] = '\0';
                    break;

                case Unknown:
                    uart_flush();
                    break;
            }
            break;
        // CTRL-C
        case 3:
            cmd[0] = '\0';
            goto shell_input_loop_end;
        // Backspace
        case 8:
        case 127:
            if (idx > 0) {
                idx--;
                // left shift command
                for (i = idx; i < end; i++) {
                    cmd[i] = cmd[i + 1];
                }
                cmd[--end] = '\0';
            }
            break;
        // Insert new character to cmd
        default:
            if (end == cmd_size) return end;
            // right shift command
            if (idx < end) {
                for (i = end; i > idx; i--) {
                    cmd[i] = cmd[i - 1];
                }
            }
            cmd[idx++] = c;
            cmd[++end] = '\0';
            break;
        }
        
        uart_write_string("\r# ");
        uart_write_string(cmd);
        for (int j = 0; j < (last_end - end); j++)
            uart_write(' ');
        uart_write_string("\r\e[");
        uart_write_no((unsigned int)(idx+2));
        uart_write_string("C");
        last_end = end;
    }
shell_input_loop_end:
    uart_write_string("\n\r");
    return (unsigned int)end;
}

unsigned int shell_set_first_arg(char *entire_cmd, unsigned int cmd_size, char *buffer, unsigned int buffer_size)
{
    while (*entire_cmd == ' ') {
        entire_cmd++;
        cmd_size--;
    }
    char *first_arg_tail = entire_cmd, *cmd_tail = entire_cmd + cmd_size;
    for (; first_arg_tail < cmd_tail; first_arg_tail++) {
        char cur = *first_arg_tail;
        if (cur == '\0' || cur == '\n') break;
    }
    unsigned int copy_size = min(buffer_size, (unsigned int)(first_arg_tail - entire_cmd));
    memcpy(buffer, entire_cmd, copy_size);
    buffer[copy_size] = '\0';
    return copy_size;
}

void shell_process_cmd(char *input_buffer, unsigned int input_size)
{
    /* split first arg and others*/
    char first_arg[MAX_SHELL_INPUT];
    input_size = min(input_size, (unsigned int)MAX_SHELL_INPUT);
    unsigned int first_arg_content_size = shell_set_first_arg(input_buffer, input_size, first_arg, MAX_SHELL_INPUT);
    
    if (!run_if_builtin(first_arg, input_buffer + first_arg_content_size)) {
        uart_write_string("Command not found!\n\r");
    }
}


int run_if_builtin(char *first_arg, char *other_args)
{
    if (strcmp(first_arg, "help") == 0) {
        uart_write_string("help   : print this help menu\n\r");
        uart_write_string("hello  : print Hello World!\n\r");
        uart_write_string("reboot : reboot the device\n\r");
        return 1;
    } else if (strcmp(first_arg, "hello") == 0) {
        uart_write_string("Hello World!\n\r");
        return 1;
    } else if (strcmp(first_arg, "reboot") == 0) {
        reset(0);
        while (1) delay(100); // hang until reboot
        return 1;
    }
    return 0;
}