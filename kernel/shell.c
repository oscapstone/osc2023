#include "shell.h"
#include "uart.h"
#include "string.h"
#include "mailbox.h"
#include "helper.h"

void print_help_menu()
{
    uart_puts("help     : print this help menu.\n");
    uart_puts("hello    : print hello, world.\n");
    uart_puts("reboot   : reboot the device.\n");
    uart_puts("revision : print board revision.\n");
    uart_puts("memory   : print memory information.\n");
    uart_puts("ls       : list files in current directory.\n");
    uart_puts("cat      : print file.\n");
}

void print_hello_world()
{
    uart_puts("hello, world\n");
}

void reset(int tick)
{
    uart_puts("rebooting...\n");
    set(PM_RSTC, PM_PASSWORD | 0x20); // full reset
    set(PM_WDOG, PM_PASSWORD | tick); // number of watchdog tick
}

void cancel_reset()
{
    set(PM_RSTC, PM_PASSWORD | 0); // full reset
    set(PM_WDOG, PM_PASSWORD | 0); // number of watchdog tick
}

void operate_command(char *command)
{
    if (string_compare(HELP, command))
    {
        print_help_menu();
    }
    else if (string_compare(HELLO, command))
    {
        print_hello_world();
    }
    else if (string_compare(REBOOT, command))
    {
        reset(100);
    }
    else if (string_compare(REVISION, command))
    {
        print_board_revision();
    }
    else if (string_compare(MEMORY, command))
    {
        print_arm_memory();
    }
    else if (string_compare(LS, command))
    {
        list_files();
    }
    else if (string_start_with(command, CAT))
    {
        print_file(command);
    }
    else if (string_length(command) == 0)
    {
        uart_newline();
    }
    else
    {
        uart_puts("command \"");
        uart_puts(command);
        uart_puts("\" not found!\n");
    }
}

void get_command(string *command)
{
    char input;
    do
    {
        input = uart_read();
        if (input == '\n')
        {
            uart_write('\r');
        }
        uart_write(input);

        string_append(command, input);
    } while (input != '\n');
}

void start_shell()
{
    string command;
    string_init(&command);

    while (1)
    {
        uart_puts(SHELL_PROMPT);

        get_command(&command);
        operate_command(string_get_string(command));

        string_clear(&command);
    }
}