#include <stddef.h>
#include "shell.h"
#include "uart1.h"
#include "mbox.h"
#include "power.h"
#include "cpio.h"
#include "u_string.h"
#include "dtb.h"
#include "buddy_system.h"
#include "stdlib.h"
#include "timer.h"

#define CLI_MAX_CMD 12
#define USTACK_SIZE 0x10000

extern char* dtb_ptr;
void* CPIO_DEFAULT_START;

unsigned long int ptr[1024] = {0};
int valid[1024] = {0};
int ptr_num = 0;

void cli_cmd_clear(char* buffer, int length)
{
    for(int i=0; i<length; i++)
    {
        buffer[i] = '\0';
    }
};

void cli_cmd_read(char* buffer)
{
    char c='\0';
    int idx = 0;
    while(1)
    {
        if ( idx >= CMD_MAX_LEN ) break;
        c = uart_async_getc();
        if ( c == '\n') break;
        buffer[idx++] = c;
    }
}

void cli_cmd_exec(char* buffer)
{
    if (!buffer) return;

    char* cmd = buffer;
    char* argvs = str_SepbySpace(buffer);

    if (strcmp(cmd, "cat") == 0) {
        do_cmd_cat(argvs);
    } else if (strcmp(cmd, "dtb") == 0){
        do_cmd_dtb();
    } else if (strcmp(cmd, "exec") == 0){
        do_cmd_exec(argvs);
    } else if (strcmp(cmd, "hello") == 0) {
        do_cmd_hello();
    } else if (strcmp(cmd, "help") == 0) {
        do_cmd_help();
    } else if (strcmp(cmd, "info") == 0) {
        do_cmd_info();
    } else if (strcmp(cmd, "kmalloc") == 0) {
        do_cmd_kmalloc();
    } else if (strcmp(cmd, "ls") == 0) {
        do_cmd_ls(argvs);
    } else if (strcmp(cmd, "setTimeout") == 0) {
        char* sec = str_SepbySpace(argvs);
        do_cmd_setTimeout(argvs, sec);
    } else if (strcmp(cmd, "set2sAlert") == 0) {
        do_cmd_set2sAlert();
    } else if (strcmp(cmd, "bs_alloc") == 0) {
        do_cmd_buddy_system_alloc(cmd);  
    } else if (strcmp(cmd, "bs_free") == 0) {
        do_cmd_buddy_system_free(cmd);  
    } else if (strcmp(cmd, "malloc") == 0) {
        do_cmd_malloc(cmd);  
    } else if (strcmp(cmd, "free") == 0) {
        do_cmd_free(cmd);  
    } else if (strcmp(cmd, "reboot") == 0) {
        do_cmd_reboot();
    }
    else{
        uart_puts("Error command!\n");
    }
}

void cli_print_banner()
{
    uart_puts("\r\n");
    uart_puts("=======================================\r\n");
    uart_puts("                 Shell                 \r\n");
    uart_puts("=======================================\r\n");
}

void do_cmd_cat(char* filepath)
{
    char* c_filepath;
    char* c_filedata;
    unsigned int c_filesize;
    struct cpio_newc_header *header_ptr = CPIO_DEFAULT_START;

    while(header_ptr!=0)
    {
        int error = cpio_newc_parse_header(header_ptr, &c_filepath, &c_filesize, &c_filedata, &header_ptr);
        //if parse header error
        if(error)
        {
            uart_puts("cpio parse error");
            break;
        }

        if(strcmp(c_filepath, filepath)==0)
        {
            uart_puts("%s", c_filedata);
            break;
        }

        //if this is TRAILER!!! (last of file)
        if(header_ptr==0) uart_puts("cat: %s: No such file or directory\n", filepath);
    }
}

void do_cmd_dtb()
{
    traverse_device_tree(dtb_ptr, dtb_callback_show_tree);
}

void do_cmd_help()
{
    uart_puts("\n");
    uart_puts("\tcat:\t\tconcatenate files and print on the standard output.\n");
    uart_puts("\tdtb:\t\tshow device tree.\n");
    uart_puts("\texec:\t\tturn program from cpio.\n");
    uart_puts("\thello:\t\tprint Hello World!.\n");
    uart_puts("\thelp:\t\tprint all available commands.\n");
    uart_puts("\tkmalloc:\tsimple allocator in heap session.\n");
    uart_puts("\tinfo:\t\tget device information via mailbox.\n");
    uart_puts("\tls:\t\tlist directory contents.\n");
    uart_puts("\tsetTimeout:\tsetTimeout [MESSAGE] [SECONDS].\n");
    uart_puts("\tset2sAlert:\tset core timer interrupt every 2 second.\n");
    uart_puts("\tbs_alloc:\t alloc buddy system in KB\n");
    uart_puts("\tbs_free:\tfree the buddy system by index\n");
    uart_puts("\tmalloc:\t\tdynamic memory allocator.\n");
    uart_puts("\tfree:\t\tfree memory allocated using malloc\n");
    uart_puts("\treboot:\t\treboot the device.\n");
    uart_puts("\n");
}

void do_cmd_exec(char* filepath)
{
    char* c_filepath;
    char* c_filedata;
    unsigned int c_filesize;
    struct cpio_newc_header *header_ptr = CPIO_DEFAULT_START;

    while(header_ptr!=0)
    {
        int error = cpio_newc_parse_header(header_ptr, &c_filepath, &c_filesize, &c_filedata, &header_ptr);
        //if parse header error
        if(error)
        {
            uart_puts("cpio parse error");
            break;
        }

        if(strcmp(c_filepath, filepath)==0)
        {
            //exec c_filedata
            char* ustack = kmalloc(USTACK_SIZE);
            asm("msr elr_el1, %0\n\t"   // elr_el1: Set the address to return to: c_filedata
                "msr spsr_el1, xzr\n\t" // enable interrupt (PSTATE.DAIF) -> spsr_el1[9:6]=4b0. In Basic#1 sample, EL1 interrupt is disabled.
                "msr sp_el0, %1\n\t"    // user program stack pointer set to new stack.
                "eret\n\t"              // Perform exception return. EL1 -> EL0
                :: "r" (c_filedata),
                   "r" (ustack+USTACK_SIZE));
            s_free(ustack);
            break;
        }

        //if this is TRAILER!!! (last of file)
        if(header_ptr==0) uart_puts("cat: %s: No such file or directory\n", filepath);
    }

}

void do_cmd_hello()
{
    uart_puts("Hello World!\r\n");
}

void do_cmd_info()
{
    // print hw revision
    pt[0] = 8 * 4;
    pt[1] = MBOX_REQUEST_PROCESS;
    pt[2] = MBOX_TAG_GET_BOARD_REVISION;
    pt[3] = 4;
    pt[4] = MBOX_TAG_REQUEST_CODE;
    pt[5] = 0;
    pt[6] = 0;
    pt[7] = MBOX_TAG_LAST_BYTE;

    if (mbox_call(MBOX_TAGS_ARM_TO_VC, (unsigned int)((unsigned long)&pt)) ) {
        uart_puts("Hardware Revision\t: ");
        uart_2hex(pt[6]);
        uart_2hex(pt[5]);
        uart_puts("\r\n");
    }
    // print arm memory
    pt[0] = 8 * 4;
    pt[1] = MBOX_REQUEST_PROCESS;
    pt[2] = MBOX_TAG_GET_ARM_MEMORY;
    pt[3] = 8;
    pt[4] = MBOX_TAG_REQUEST_CODE;
    pt[5] = 0;
    pt[6] = 0;
    pt[7] = MBOX_TAG_LAST_BYTE;

    if (mbox_call(MBOX_TAGS_ARM_TO_VC, (unsigned int)((unsigned long)&pt)) ) {
        uart_puts("ARM Memory Base Address\t: ");
        uart_2hex(pt[5]);
        uart_puts("\r\n");
        uart_puts("ARM Memory Size\t\t: ");
        uart_2hex(pt[6]);
        uart_puts("\r\n");
    }
}

void do_cmd_kmalloc()
{
    //test malloc
    char* test1 = kmalloc(0x18);
    memcpy(test1,"test malloc1",sizeof("test malloc1"));
    uart_puts("%s\n",test1);

    char* test2 = kmalloc(0x20);
    memcpy(test2,"test malloc2",sizeof("test malloc2"));
    uart_puts("%s\n",test2);

    char* test3 = kmalloc(0x28);
    memcpy(test3,"test malloc3",sizeof("test malloc3"));
    uart_puts("%s\n",test3);
}

void do_cmd_ls(char* workdir)
{
    char* c_filepath;
    char* c_filedata;
    unsigned int c_filesize;
    struct cpio_newc_header *header_ptr = CPIO_DEFAULT_START;

    while(header_ptr!=0)
    {
        int error = cpio_newc_parse_header(header_ptr, &c_filepath, &c_filesize, &c_filedata, &header_ptr);
        //if parse header error
        if(error)
        {
            uart_puts("cpio parse error");
            break;
        }

        //if this is not TRAILER!!! (last of file)
        if(header_ptr!=0) uart_puts("%s\n", c_filepath);
    }
}

void do_cmd_setTimeout(char* msg, char* sec)
{
    add_timer(uart_sendline,atoi(sec),msg);
}

void do_cmd_set2sAlert()
{
    add_timer(timer_set2sAlert,2,"2sAlert");
}

void do_cmd_buddy_system_alloc(char *command)
{
    uart_puts("size(KB): ");
    cli_cmd_read(command);
    unsigned long int address = buddy_system_alloc(atoi(command));
    uart_puts("address: 0x");
    uart_2hex(address);
    uart_puts("\n");
}

void do_cmd_buddy_system_free(char *command)
{
    uart_puts("index: ");
    cli_cmd_read(command);
    //uart_sendline("atoi :%d\n", hex2dec(command));
    buddy_system_free(hex2dec(command));
    uart_puts("\n");
}

void do_cmd_malloc(char *command)
{
    uart_puts("size(B): ");
    cli_cmd_read(command);
    ptr[ptr_num] = (unsigned long int)malloc(atoi(command));
    uart_puts("index: ");
    uart_2hex(ptr_num);
    uart_puts("\n");
    uart_puts("address: 0x");
    uart_2hex(ptr[ptr_num]);
    uart_puts("\n");
    valid[ptr_num] = 1;
    ptr_num++;
}

void do_cmd_free(char *command)
{
    uart_puts("index: ");
    cli_cmd_read(command);
    if (valid[atoi(command)]) {
        free_page((unsigned long int)ptr[hex2dec(command)]);
        valid[atoi(command)] = 0;
    } else
        uart_puts("invalid index\n");
}

void do_cmd_reboot()
{
    uart_puts("Reboot in 5 seconds ...\r\n\r\n");
    volatile unsigned int* rst_addr = (unsigned int*)PM_RSTC;
    *rst_addr = PM_PASSWORD | 0x20;
    volatile unsigned int* wdg_addr = (unsigned int*)PM_WDOG;
    *wdg_addr = PM_PASSWORD | 5;
}

