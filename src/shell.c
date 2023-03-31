#include "dtb.h"
#include "uart.h"
#include "utils.h"
#include "reboot.h"
#include "shell.h"
#include "note.h"
#include "initramfs.h"
#include "time_interrupt.h"

enum ANSI_ESC
{
    Unknown,
    CursorForward,
    CursorBackward,
    Delete
};

enum ANSI_ESC decode_csi_key()
{
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

enum ANSI_ESC decode_ansi_escape()
{
    return uart_read() == '[' ? decode_csi_key() : Unknown;
}

unsigned int shell_read_string(char *cmd, unsigned int cmd_size)
{
    uart_write_string("# ");

    int idx = 0, end = 0, i, last_end = 0;
    cmd[0] = '\0';
    char c;
    while ((c = uart_read()) != '\n')
    {
        // Decode CSI key sequences
        if (c == 27) {
            enum ANSI_ESC key = decode_ansi_escape();
            switch (key)
            {
            case CursorForward:
                if (idx < end)
                    idx++;
                break;

            case CursorBackward:
                if (idx > 0)
                    idx--;
                break;

            case Delete:
                // left shift command
                for (i = idx; i < end; i++)
                {
                    cmd[i] = cmd[i + 1];
                }
                cmd[--end] = '\0';
                break;

            case Unknown:
                uart_flush();
                break;
            }
        }
        // CTRL-C
        else if(c == 3) {
            cmd[0] = '\0';
            goto shell_input_loop_end;
        }
        else if (c == 8 || c == 127) {
        // Backspace
            if (idx > 0)
            {
                idx--;
                // left shift command
                for (i = idx; i < end; i++)
                {
                    cmd[i] = cmd[i + 1];
                }
                cmd[--end] = '\0';
            }
        }
        // Insert new character to cmd
        else {
            if (end == cmd_size)
                return end;
            // right shift command
            if (idx < end)
            {
                for (i = end; i > idx; i--)
                {
                    cmd[i] = cmd[i - 1];
                }
            }
            cmd[idx++] = c;
            cmd[++end] = '\0';
        }

        uart_write_retrace();
        uart_write_string("# ");
        uart_write_string(cmd);
        for (int j = 0; j < (last_end - end); j++)
            uart_write(' ');
        uart_write_retrace();
        uart_write_string("\e[");
        uart_write_no((unsigned int)(idx + 2));
        uart_write_string("C");
        last_end = end;
    }
shell_input_loop_end:
    uart_write_string("\n");
    return (unsigned int)end;
}

unsigned int shell_set_first_arg(char *entire_cmd, unsigned int cmd_size, char *buffer, unsigned int buffer_size)
{
    while (*entire_cmd == ' ')
    {
        entire_cmd++;
        cmd_size--;
    }
    char *first_arg_tail = entire_cmd, *cmd_tail = entire_cmd + cmd_size;
    for (; first_arg_tail < cmd_tail; first_arg_tail++)
    {
        char cur = *first_arg_tail;
        if (cur == '\0' || cur == '\n')
            break;
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

    if (!run_if_builtin(first_arg, input_buffer + first_arg_content_size))
    {
        uart_write_string("Command not found!\n");
    }
}

int run_if_builtin(char *first_arg, char *other_args)
{
    char input_buffer[MAX_SHELL_INPUT];
    char *argv[MAX_ARGS+1];
    if (strcmp(first_arg, "help") == 0)
    {
        uart_write_string("ls              : list files in initramfs\n");
        uart_write_string("cat             : print content of a specific file in initramfs\n");
        uart_write_string("dtb             : print device tree properties\n");
        uart_write_string("help            : print this help menu\n");
        uart_write_string("hello           : print Hello World!\n");
        uart_write_string("async           : read and write a line asynchronously\n");
        uart_write_string("reboot          : reboot the device\n");
        uart_write_string("mknote          : make a note\n");
        uart_write_string("linote          : list all notes\n");
        uart_write_string("ldprog          : run program in EL0\n");
        uart_write_string("uptime          : write system uptime\n");
        uart_write_string("setTimeout      : write message at the given time\n");
        uart_write_string("test_preemption : sleep the given amount of time and enqueue three timer task to timer queue.\n");
        return 1;
    }
    else if (strcmp(first_arg, "hello") == 0)
    {
        uart_write_string("Hello World!\n");
        return 1;
    }
    else if (strcmp(first_arg, "reboot") == 0)
    {
        reset(0);
        while (1)
            delay(10); // hang until reboot
        return 1;
    }
    else if (strcmp(first_arg, "ls") == 0)
    {
        //currently only support root directory listing
        _initramfs.ls(&_initramfs, "");
        return 1;
    }
    else if (strcmp(first_arg, "cat") == 0)
    {
        uart_write_string("Filename: ");
        uart_read_input(input_buffer, MAX_SHELL_INPUT);
        _initramfs.cat(&_initramfs, input_buffer);
        return 1;
    }
    else if (strcmp(first_arg, "dump") == 0) 
    {
        uart_write_string("64b hex address: ");
        dump_hex(uart_read_hex_ull(), 8);
        return 1;
    }
    else if (strcmp(first_arg, "mknote") == 0) 
    {
        make_note();
        return 1;
    }
    else if (strcmp(first_arg, "linote") == 0)
    {
        list_note();
        return 1;
    }
    else if (strcmp(first_arg, "dtb") == 0)
    {
        _fdt.fdt_print(&_fdt);
        return 1;
    }
    else if (strcmp(first_arg, "ldprog") == 0)
    {
        uart_write_string("args: ");
        uart_read_input(input_buffer, MAX_SHELL_INPUT);
        shell_parse_argv(input_buffer, argv, MAX_ARGS);
        _initramfs.exec(&_initramfs, argv);
        return 1;
    }
    else if (strcmp(first_arg, "uptime") == 0)
    {
        write_uptime();
        return 1;
    } else if (strcmp(first_arg, "async") == 0) {
        test_uart_async();
        return 1;
    } else if (strcmp(first_arg, "setTimeout") == 0) {
        uart_write_string("message length: ");
        uart_read_input(input_buffer, MAX_SHELL_INPUT);
        int buf_size = atoi(input_buffer);
        if (buf_size <= 0 || buf_size > 100) {
            uart_write_string("length should within [1, 100].\n");
            return 2;
        }
        char *msg = (char *)simple_malloc(buf_size * sizeof(char));
        uart_write_string("message: ");
        uart_read_input(msg, buf_size);
        uart_write_string("duration: ");
        uart_read_input(input_buffer, MAX_SHELL_INPUT);
        int dur = atoi(input_buffer);
        if (dur <= 0 || dur > 100) {
            uart_write_string("duration should within [1, 100].\n");
            return 2;
        }
        _timer_task_scheduler.add_timer_second(&_timer_task_scheduler, notify, msg, dur);
        return 1;
    } else if (strcmp(first_arg, "test_preemption") == 0) {
        // uart_write_string("t: ");
        // uart_read_input(input_buffer, MAX_SHELL_INPUT);
        // int t = atoi(input_buffer);
        // if (t <= 0 || t > 100) {
        //     uart_write_string("t should within [1, 100].\n");
        //     return 2;
        // }
        // _timer_task_scheduler.add_timer_second(&_timer_task_scheduler, sleep_timer, (size_t)t, 1);
        _timer_task_scheduler.add_timer_second(&_timer_task_scheduler, sleep_timer, 1, 1);
        return 1;
    }
    return 0;
}

int shell_parse_argv(char *s, char *argv[], unsigned max_args)
{
    char *token = strtok(s, " \n");
    int argc = 0;
    while (token != NULL && argc < max_args) {
        argv[argc++] = token;
        token = strtok(NULL, " \n");
    }
    argv[argc] = NULL;
    return argc;
}

void shell_main(void)
{
    /* shell */
    unsigned int read_cnt;
    char input_buffer[MAX_SHELL_INPUT];

    while (1) {
        read_cnt = shell_read_string(input_buffer, MAX_SHELL_INPUT);
        shell_process_cmd(input_buffer, read_cnt);
    }

}