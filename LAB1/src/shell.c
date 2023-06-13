#include "shell.h"
#include "mini_uart.h"
#include "peripherals/reboot.h"

// because of the implementation of uart_send()
// char *help_info = "help\t\t: print this help menu\nhello\t\t: print Hello World!\nreboot\t\t: reboot the device\n";
// char *hello_info = "Hello, world!\n"; // assign string to char array is no need to put \0 behind extra
char help_info[] = "help\t\t: print this help menu\nhello\t\t: print Hello World!\nreboot\t\t: reboot the device\n";
char hello_info[] = "Hello, world!\n"; // assign string to char array is no need to put \0 behind extra
char reboot_info[] = "----------------system reboot after 5 sec----------------\n";
char input_str[10];

/* Compare S1 and S2, returning less than, equal to or
    greater than zero if S1 is lexicographically less than,
    equal to or greater than S2.  */
int strcmp(const char *p1, const char *p2)
{
    const unsigned char *s1 = (const unsigned char *)p1;
    const unsigned char *s2 = (const unsigned char *)p2;
    unsigned char c1, c2;
    do
    {
        c1 = (unsigned char)*s1++;
        c2 = (unsigned char)*s2++;
        if (c1 == '\0')
            return c1 - c2;
    } while (c1 == c2);
    return c1 - c2;
}
void shell_run(char input_ch)
{
    static int cnt = 0;
    static int i = 0;
    if (input_ch == '\n')
    {
        // if (!strcmp("help",input_str))
        if (!strcmp(input_str, "help"))
        {
            uart_send_string(help_info);
            // uart_send('1');
        }
        else if (!strcmp("hello", input_str))
        {
            uart_send_string(hello_info);
            // uart_send('2');
        }
        else if (!strcmp("reboot", input_str))
        {
            uart_send_string(reboot_info);
            reset(5 << 16); // 1 sec = 2^16 ticks
            // uart_send('3');
        }
        cnt = 0;
        for (i = 0; i < 10; i++)
            input_str[i] = '\0';
        // uart_send('5');
    }
    // still not press the enter
    else if (cnt < 9)
    {
        input_str[cnt++] = input_ch;
    }
    else
    {
        cnt = 0;
        for (i = 0; i < 10; i++)
            input_str[i] = '\0';
    }
}
