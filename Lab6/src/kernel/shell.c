#include "stdlib.h"
#include "mini_uart.h"
#include "reboot.h"
#include "read_cpio.h"
#include "device_tree.h"
#include "timer.h"
#include "page_alloc.h"
#include "dynamic_alloc.h"
#include "test.h"
#include "thread.h"

extern void *_dtb_ptr;
extern char *cpioDestGlobal;
extern char read_buffer[100];
// extern page_frame_node frame_array[TOTAL_NUM_PAGE];
// extern chunk chunk_array[3000];

#define COMMAND_BUFFER 50
#define FILENAME_BUFFER 20
#define ARGV_BUFFER 30

void shell_start();

void shell_main(char *command)
{
    if (!strcmp(command, "help"))
    {
        uart_send_string("help\t: print this help menu\n");
        uart_send_string("hello\t: print Hello World!\n");
        uart_send_string("reboot\t: reboot the device\n");
        uart_send_string("ls\t: list information about files\n");
        uart_send_string("cat\t: copy each FILE to standard output\n");
        uart_send_string("dts\t: list devicetree\n");
        uart_send_string("asynr\t: [test] asynchronous read\n");
        uart_send_string("asynw\t: [test] asynchronous write\n");
        uart_send_string("setTimeout\t: Usage: setTimeout <Message> <Seconds>\n");
        uart_send_string("alloc\t: [test] malloc and free\n");
        uart_send_string("thread\t: [test]\n");
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
        read_cpio((char *)cpioDestGlobal);
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

        read_content((char *)cpioDestGlobal, filename);
    }
    else if (!strcmp(command, "dts"))
    {
        fdt_traverse(dtb_parser, _dtb_ptr);
    }
    else if (!strcmp(command, "time"))
    {
        get_current_time();
        asm volatile(
            "mov x1, 0x0;" // not sure why can't use x0, may have something to to with %0
            "msr spsr_el1, x1;"
            "mov x2, %0;"
            "add x2, x2, 12;"
            "msr elr_el1, x2;"
            "mov x2, 0x15000000;"
            "msr sp_el0, x2;"
            "mrs x2, cntfrq_el0;"
            "add x2, x2, x2;"
            "msr cntp_tval_el0, x2;"
            "bl core_timer_enable;"
            "eret;"
            :
            : "r"(shell_start)
            :);
    }
    else if (!strcmp(command, "asynr"))
    {
        asyn_read();
    }
    else if (!strcmp(command, "asynw"))
    {
        asyn_write(read_buffer);
        uart_send('\n');
    }
    else if (!memcmp(command, "setTimeout", 10))
    {
        if (command[10] != ' ' || command[11] == '\0')
        {
            printf("Usage: setTimeout <Message> <Seconds>\n");
            return;
        }

        char message[MESSAGE_BUFFER];
        memset(message, '\0', MESSAGE_BUFFER);
        int i = 11;
        while (command[i] != ' ' && command[i] != '\0')
        {
            message[i - 11] = command[i];
            i++;
        }

        if (command[i] != ' ' || command[i] == '\0' || command[i + 1] == '\0')
        {
            printf("Usage: setTimeout <Message> <Seconds>\n");
            return;
        }

        char seconds[SECONDS_BUFFER];
        memset(seconds, '\0', SECONDS_BUFFER);
        while (command[i] != '\0')
        {
            seconds[i - strlen(message) - 1 - 11] = command[i];
            i++;
        }
        int sec;
        sec = atoi(seconds);

        add_timer(sec, message);
    }
    else if (!memcmp(command, "alloc", 5))
    {
        test_mem_alloc();
    }
    else if (!memcmp(command, "debug", 5))
    {
        debug();
        debug_pool();
    }
    else if (!memcmp(command, "thread", 6))
    {
        test_thread();
    }
    else if (!strcmp(command, "syscall"))
    {
        thread_create(load_usrpgm_in_umode);
        idle_task();
    }

    return;
}

void shell_start()
{
    uart_send_string("Starting shell...\n");
    char c;
    int i = 0;

    char command[COMMAND_BUFFER];
    memset(command, '\0', COMMAND_BUFFER);

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
