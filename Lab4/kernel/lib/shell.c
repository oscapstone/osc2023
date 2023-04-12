#include "uart.h"
#include "mbox.h"
#include "string.h"
#include "malloc.h"
#include "cpio.h"
#include "exec.h"
#include "timer.h"
#include "interrupt.h"
#include "task.h"
#include "memory.h"

extern char *cpio_start;

/* mbox */
void get_board_revision(){
    mbox[0] = 7 * 4; // buffer size in bytes
    mbox[1] = MBOX_REQUEST;
    // tags begin
    mbox[2] = 0x00010002; // tag identifier : GET_BOARD_REVISION
    mbox[3] = 4; // maximum of request and response value buffer's length.
    mbox[4] = 4;
    mbox[5] = 0; // value buffer
    // tags end
    mbox[6] = MBOX_TAG_LAST;

    mbox_call(MBOX_CH_PROP); // message passing procedure call, you should implement it following the 6 steps provided above.

    uart_puts("board version:    0x");
    uart_hex(mbox[5]);
    uart_puts("\n");
}

void get_arm_memory(){
    mbox[0] = 8 * 4; // buffer size in bytes
    mbox[1] = MBOX_REQUEST;
    // tags begin
    mbox[2] = 0x00010005; // tag identifier: GET_ARM_MEMORY
    mbox[3] = 8; // maximum of request and response value buffer's length.
    mbox[4] = 8;
    mbox[5] = 0; // value buffer
    mbox[6] = 0; // value buffer
    // tags end
    mbox[7] = MBOX_TAG_LAST;

    mbox_call(MBOX_CH_PROP); // message passing procedure call, you should implement it following the 6 steps provided above.

    uart_puts("Arm base address: 0x");
    uart_hex(mbox[5]);
    uart_puts("\n");
    uart_puts("Arm memory size:  0x");
    uart_hex(mbox[6]);
    uart_puts("\n");
}

/* reboot */
#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024

void set(long addr, unsigned int value) {
    volatile unsigned int* point = (unsigned int*)addr;
    *point = value;
}

void reset(int tick) {                 // reboot after watchdog timer expire
    set(PM_RSTC, PM_PASSWORD | 0x20);  // full reset
    set(PM_WDOG, PM_PASSWORD | tick);  // number of watchdog tick
}

void cancel_reset() {
    set(PM_RSTC, PM_PASSWORD | 0);  // full reset
    set(PM_WDOG, PM_PASSWORD | 0);  // number of watchdog tick
}

/* cpio */
int cat(char *thefilepath)
{
    char *filepath;
    char *filedata;
    unsigned int filesize;
    struct cpio_newc_header *header_pointer = (struct cpio_newc_header *)cpio_start;

    while (header_pointer)
    {
        int error = cpio_newc_parse_header(header_pointer, &filepath, &filesize, &filedata, &header_pointer);
        // if parse header error
        if (error)
        {
            uart_printf("error\n");
            break;
        }

        if (!strcmp(thefilepath, filepath))
        {
            for (unsigned int i = 0; i < filesize; i++)
                uart_printf("%c", filedata[i]);
            uart_printf("\n");
            break;
        }

        if (header_pointer == 0)
            uart_printf("cat: %s: No such file or directory\r\n", thefilepath);
    }
    return 0;
}

int ls(char *working_dir)
{
    char *filepath;
    char *filedata;
    unsigned int filesize;
    struct cpio_newc_header *header_pointer = (struct cpio_newc_header *)cpio_start;

    while (header_pointer)
    {
        int error = cpio_newc_parse_header(header_pointer, &filepath, &filesize, &filedata, &header_pointer);
        // if parse header error
        if (error)
        {
            uart_printf("error\n");
            break;
        }

        // if this is not TRAILER!!! (last of file)
        if (header_pointer != 0)
            uart_printf("%s\n", filepath);
    }
    return 0;
}

/* timer */
void print_timeout(char * str) {
    uart_printf("setTimeout  Current second: %d\tmsg: %s\n", get_clock_time(), str);
}

void two_second(char * str) {
    uart_printf("twoSec      Current second: %d\n", get_clock_time());
    add_timer(two_second, "", get_current_tick() + 2 * get_clock_freq());
}

/* preempt */
void high_priority_task() {
    uart_async_printf("high priority start\n");
    uart_async_printf("high priority end\n");
}

void low_priority_task() {
    uart_async_printf("low priority start\n");
    add_task(high_priority_task, 1);
    uart_async_printf("\r"); // trigger pop_task
    for (int i = 0; i < 1000000; i++) ;
    uart_async_printf("low priority end\n");
}

void test_preemption() {
    add_task(low_priority_task, 9);
    uart_async_printf("\r"); // trigger pop_task
}

/* shell */
void shell(void) {
    uart_puts("! Welcome Lab4 !\n");

    char command[32];
    int idx = 0;

    while (1) {
        idx = 0;
        uart_puts("$ ");
        while (1) {
            command[idx] = uart_getc();
            if (command[idx] == '\n') {
                command[idx] = '\0';
                break;
            }
            idx++;
        }

        // Lab 1
        if (strcmp("hello", command) == 0) {
            uart_puts("Hello Kernel!\n");
        }
        else if (strcmp("help", command) == 0) {
            uart_puts("help                   : print this help menu\n");
            uart_puts("hello                  : print Hello World!\n");
            uart_puts("reboot                 : reboot this device\n");
            uart_puts("lshw                   : print hardware info from mailbox\n");
            uart_puts("malloc                 : allocate memory to specific string\n");
            uart_puts("ls                     : print files and directories\n");
            uart_puts("cat [FILE]             : print FILE\n");
            uart_puts("exec                   : execute file to go to EL1\n");
            uart_puts("preempt                : test preemption\n");
            uart_puts("setTimeout [MSG] [SEC] : prints MSG after SEC\n");
            uart_puts("twoSec                 : enable two seconds alert\n");
            uart_puts("mmp                    : test page frame malloc\n");
            uart_puts("mmc                    : test chunk malloc\n");
        }
        else if (strcmp("reboot", command) == 0) {
            uart_puts("rebooting...\n");
            reset(1000);
        }
        else if (strcmp("lshw", command) == 0) {
            get_board_revision();
            get_arm_memory();
        }
        // Lab 2
        else if (strcmp("malloc", command) == 0) {
            uart_puts("allocating...\n");
            char* str = (char*) simple_malloc(8);
            *str = 'a';
            *(str + 1) = 'b';
            *(str + 2) = 'c';
            *(str + 3) = '\0';
            uart_printf("%s\n", str);
        }
        else if (strcmp("ls", command) == 0) {
            ls(".");
        }
        else if (strncmp("cat", command, 3) == 0) {
            cat(command + 4);
        }
        // Lab 3
        else if (strcmp("exec", command) == 0) {
            exec_file("test");
        }
        else if (strcmp("async", command) == 0) {
            char c = 'a';
            uart_printf("Press `q` to quit\n");
            while (c != 'q') {
                c = uart_async_getc();
                uart_async_putc(c);
            }
            uart_printf("\n");
        }
        else if (strcmp("preempt", command) == 0) {
            test_preemption();
        }
        else if (strncmp("setTimeout", command, 10) == 0) {
            int idx = 11;
            char* msg = (char*) simple_malloc(15 * sizeof(char));
            int msg_idx = 0;
            while (command[idx] != ' ') {
                msg[msg_idx++] = command[idx++];
            }
            msg[msg_idx] = '\0';
            int sec = atoi(command + idx + 1);
            uart_printf("setTimeout %d seconds start at %d\n", sec, get_clock_time());
            add_timer(print_timeout, msg, get_current_tick() + sec * get_clock_freq());
        }
        else if (strcmp("twoSec", command) == 0) {
            add_timer(two_second, "", get_current_tick() + 2 * get_clock_freq());
        }
        // Lab 4
        else if (strcmp("mmp", command) == 0) {
            page_frame_allocator_test();
        }
        else if (strcmp("mmc", command) == 0) {
            chunk_slot_allocator_test();
        }
        else {
            uart_puts("unknown\n");
        }
    }
}

