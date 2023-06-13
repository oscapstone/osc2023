#include "shell.h"
#include "mini_uart.h"
#include "utils.h"
#include "mailbox.h"
#include "reboot.h"
#include "string.h"
#include "cpio.h"
#include "memory.h"
#include "timer.h"
#include "exception.h"
#include "mm.h"

#define MAX_BUFFER_SIZE 256u

static char buffer[MAX_BUFFER_SIZE];
static void* to_free[10] = {(void*)MEM_REGION_BEGIN};
static void* to_freec[4] = {(void*)MEM_REGION_BEGIN};

void read_cmd()
{
    unsigned int idx = 0;
    char c = '\0';
    
    while (1) {
        c = uart_recv();
        if (c == '\r' || c == '\n') {
            uart_send_string("\n");
            
            if (idx < MAX_BUFFER_SIZE) buffer[idx] = '\0';
            else buffer[MAX_BUFFER_SIZE-1] = '\0';
            
            break;
        } else {
            uart_send(c);
            buffer[idx++] = c;
        } 
    }

}

void parse_cmd()
{

    if (stringcmp(buffer, "\0") == 0) 
        uart_send_string("\n");
    else if (stringcmp(buffer, "hello") == 0)
        uart_send_string("Hello World!\n");
    else if (stringcmp(buffer, "reboot") == 0) {
        uart_send_string("rebooting...\n");
        reset(100);
    }
    else if (stringcmp(buffer, "hwinfo") == 0) {
        get_board_revision();
        get_arm_memory();
    }
    else if (stringcmp(buffer, "ls") == 0) {
        cpio_ls();
    }
    else if (stringcmp(buffer, "cat") == 0) {
        cpio_cat();
    }
    else if (stringcmp(buffer, "test_smalloc") == 0) {
        char *str = simple_malloc(8);
        for (int i=0; i<8; i++) {
            str[i] = 'A' + i;
        }
        str[7] = '\0';
        uart_send_string(str);
        uart_send('\n');
    }
    else if (stringcmp(buffer, "execute") == 0) {
        // EL0
        cpio_exec();
    }
    else if (stringcmp(buffer, "malloc") == 0) {
        for (int i=0; i<10; i++) {
            if(i < 5) to_free[i] = malloc(4095);
            else to_free[i] = malloc(4096*pow(2, i-4));
        }
        malloc(4096*64);
    }
    /*
    page 0~4 -> 0x0111
    page 5   -> 0x2000
    page 6   -> 0x4000
    page 7   -> 0x8000
    page 8   -> 0x10000
    page 9   -> 0x20000
    */
    else if (stringcmp(buffer, "free") == 0) {
        for (int i=0; i<10; i++) {
            free(to_free[i]);
        }
    }
    else if (stringcmp(buffer, "chunk_malloc") == 0) {
        to_freec[0] = chunk_alloc(8);
        to_freec[1] = chunk_alloc(8);
        to_freec[2] = chunk_alloc(96); 
        to_freec[3] = chunk_alloc(256); 
    }
    else if (stringcmp(buffer, "chunk_free") == 0) {
        for (int i=0; i<4; i++) {
            chunk_free(to_freec[i]);
        }
    }
    else if (stringcmp(buffer, "help") == 0) {
        uart_send_string("help:\t\tprint list of available commands\n");
        uart_send_string("hello:\t\tprint Hello World!\n");
        uart_send_string("reboot:\t\treboot device\n");
        uart_send_string("hwinfo:\t\tprint hardware information\n");
        uart_send_string("ls:\t\tlist initramfs files\n");
        uart_send_string("cat:\t\tprint file content in initramfs\n");
        uart_send_string("test_smalloc:\ttest simple malloc\n");
        uart_send_string("execute:\trun program from cpio\n");
        uart_send_string("malloc:\tmalloc pages for assigned size\n");
        uart_send_string("free:\tfree pages\n");
        uart_send_string("chunk_malloc:\tdynamic allocate pages from pool\n");
        uart_send_string("chunk_free:\tfree pages back to pool\n");
    }
    else 
        uart_send_string("Command not found! Type help for commands.\n");

}

void shell_loop() 
{
    while (1) {
        uart_send_string(">> ");
        read_cmd();
        parse_cmd();
    }
}