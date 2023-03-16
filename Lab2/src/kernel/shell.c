#include "stdlib.h"
#include "mini_uart.h"
#include "reboot.h"
#include "read_cpio.h"
#include "device_tree.h"

extern void *_dtb_ptr;
extern char *cpioDest;

#define COMMAND_BUFFER 20
#define FILENAME_BUFFER 20

void shell_main(char *command)
{
    if (!strcmp(command, "help"))
    {
        uart_send_string("help\t: print this help menu\n");
        uart_send_string("hello\t: print Hello World!\n");
        uart_send_string("reboot\t: reboot the device\n");
        uart_send_string("ls\t:\n");
        uart_send_string("cat\t:\n");
        uart_send_string("dts\t:\n");
    }
    else if (!strcmp(command, "hello"))
    {
        uart_send_string("Hello World!\n");
    }
    else if (!strcmp(command, "reboot"))
    {
        uart_send_string("Rebooting in 3 seconds\n");
        reset(3 << 16);
        while (1)
            ;
    }
    else if (!strcmp(command, "ls"))
    {
        // char *cpioDest = (char *)0x8000000;
        read_cpio((char *)cpioDest);
    }
    else if (!memcmp(command, "cat", 3))
    {
        if (command[3] != ' ' || command[4] == '\0')
        {
            printf("Usage: cat <filename>\n");
            return;
        }

        char filename[FILENAME_BUFFER];
        memset(filename, '\0', FILENAME_BUFFER);
        int i = 4;
        while (command[i] != '\0')
        {
            filename[i - 4] = command[i];
            i++;
        }
        filename[i] = '\0';

        read_content((char *)cpioDest, filename);
    }
    else if (!strcmp(command, "cat"))
    {
        uart_send_string("Filename: ");

        char c;
        int i = 0;
        char filename[FILENAME_BUFFER];
        // char *cpioDest = (char *)0x8000000;

        memset(filename, '\0', FILENAME_BUFFER);

        while (1)
        {
            c = uart_recv();

            if (c >= 0 && c < 128) // Legal
            {
                if (c == '\n') // Enter
                {
                    filename[i] = '\0';
                    uart_send(c);
                    read_content((char *)cpioDest, filename);
                    break;
                }
                else if (c == 8) // Backspace
                {
                    uart_send(c);
                    uart_send(' ');
                    uart_send(c);
                    if (i > 0)
                        i--;
                }
                else
                {
                    if (i < FILENAME_BUFFER)
                    {
                        if (c == 0)
                            continue;
                        filename[i++] = c;
                    }
                    uart_send(c);
                }
            }
        }
    }
    else if (!strcmp(command, "dts"))
    {
        fdt_traverse(dtb_parser, _dtb_ptr);
    }
}

void shell_start()
{
    char c;
    int i = 0;
    char command[COMMAND_BUFFER];

    memset(command, '\0', COMMAND_BUFFER);

    uart_send_string("Starting shell...\n");
    uart_send_string("# ");

    while (1)
    {
        c = uart_recv();

        if (c >= 0 && c < 128) // Legal
        {
            if (c == '\n') // Enter
            {
                command[i] = '\0';
                uart_send(c);
                shell_main(command);
                memset(command, '\0', COMMAND_BUFFER);
                i = 0;
                uart_send_string("# ");
            }
            else if (c == 8) // Backspace
            {
                uart_send(c);
                uart_send(' ');
                uart_send(c);
                if (i > 0)
                    i--;
            }
            else if (c == 127) // Also backspace but delete
            {
            }
            else
            {
                if (i < COMMAND_BUFFER)
                {
                    if (c == 0) // solve the problem that first command on board wont work
                        continue;
                    command[i++] = c;
                }
                uart_send(c);
            }
        }
    }
}
