#include "shell.h"
#include "mini_uart.h"
#include "utils.h"
#include "mailbox.h"
#include "reboot.h"
#include "string.h"
#include "../include/cpio.h"
#include "memory.h"
#include "timer.h"
#include "exception.h"
#include "math.h"
#include "mm.h"
#include "../include/sched.h"
#include "syscall.h"
#include "peripherals/mailbox.h"
#include "fork.h"
#include "mmu.h"


#define MAX_BUFFER_SIZE 256u

static char buffer[MAX_BUFFER_SIZE];

void start_video() {

    struct cpio_newc_header *header;
    unsigned int filesize;
    unsigned int namesize;
    unsigned int offset;
    char *filename;
    void *code_loc;

    header = DEVTREE_CPIO_BASE;
    printf("devtree base: 0x%x\n", DEVTREE_CPIO_BASE);
    while (1) {
        
        filename = ((void*)header) + sizeof(struct cpio_newc_header);
        
        if (stringncmp((char*)header, CPIO_HEADER_MAGIC, 6) != 0) {
            uart_send_string("invalid magic\n");
            break;
        }
        if (stringncmp(filename, CPIO_FOOTER_MAGIC, 11) == 0) {
            uart_send_string("file does not exist!\n");
            break;
        }

        namesize = hexstr_to_uint(header->c_namesize, 8);
        
        offset = sizeof(struct cpio_newc_header) + namesize;
        if (offset % 4 != 0) 
            offset = ((offset/4) + 1) * 4;

        filesize = hexstr_to_uint(header->c_filesize, 8);

        if (stringncmp(filename, "vm.img", namesize) == 0) {
            code_loc = ((void*)header) + offset;
            break;
        }

        if (filesize % 4 != 0)
            filesize = ((filesize/4) + 1) * 4;

        offset = offset + filesize;

        header = ((void*)header) + offset;
        
    }
    printf("vm.img found in cpio at location 0x%x.\n", code_loc);
    printf("vm.img has size of %d bytes.\n", (int)filesize);
    
    int err = move_to_user_mode((unsigned long)code_loc, filesize, 0);
    if (err<0) 
        printf("failed to start video program\n");

}

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
    else if (stringcmp(buffer, "execute") == 0) {
        cpio_exec();
    }
    else if (stringcmp(buffer, "video") == 0) {
        preempt_disable();
        current->state = TASK_STOPPED;
        unsigned long long tmp;
        asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
        tmp |= 1;
        asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));
        copy_process(PF_KTHREAD, (unsigned long)&start_video, 0/*, 0*/);
        preempt_enable();
    }
    else if (stringcmp(buffer, "help") == 0) {
        uart_send_string("help:\t\tprint list of available commands\n");
        uart_send_string("hello:\t\tprint Hello World!\n");
        uart_send_string("reboot:\t\treboot device\n");
        uart_send_string("hwinfo:\t\tprint hardware information\n");
        uart_send_string("ls:\t\tlist initramfs files\n");
        uart_send_string("cat:\t\tprint file content in initramfs\n");
        uart_send_string("execute:\trun program from cpio\n");
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