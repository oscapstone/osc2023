#include <mini_uart.h>
#include <reboot.h>
#include <mbox.h>
#include <builtin.h>
#include <utils.h>
#include <cpio.h>
#include <type.h>
#include <dt17.h>
#include <string.h>
#include <mem.h>
#include <exec.h>
#include <mm.h>
#include <timer.h>
#include <current.h>
#include <kthread.h>
#include <sched.h>

static void timeout_print(char *str)
{
    uart_printf("%s\r\n", str);

    kfree(str);
}

void _help(int mode){
    uart_printf(
        "help\t: "   "print this help menu" "\r\n"
        "hello\t: "  "print Hello World!"   "\r\n"
        "hwinfo\t: " "get hardware information" "\r\n"
        "reboot\t: " "reboot the device" "\r\n"
    );
    if(mode)
        uart_printf("load\t: " "load the kernel" "\r\n");
    else{
        uart_printf(
            "ls\t: " "list files in initramfs"  "\r\n"
            "cat <filename>\t: " "get file content"  "\r\n"
            "parsedtb\t: " "parse device tree" "\r\n"
            "malloc <size>\t: " "allocate a block of memory with size" "\r\n"
            "exec <filename>\t: " "execute user program" "\r\n"
            "chmod_uart\t: " "change uart async/sync mode" "\r\n"
            "chmod_timer\t: " "change timer show/not to show mode" "\r\n"
            "setTimeout <msg> <sec>\t: " "print @msg after @sec seconds" "\r\n"
            "thread_test\t: " "test kernel thread" "\r\n"
        );
    }
}

void _hello(void){
    uart_printf("Hello World!\r\n");
}

void _hwinfo(void){
    unsigned int recv;
    arm_info arm_mem;
    get_board_revision(&recv);
    get_arm_memory(&arm_mem);
    if(recv!=0)
        uart_printf("[*] Revision: %x\r\n", recv);
    
    uart_printf("[*] ARM memory base address: %x\r\n", arm_mem.base_addr);
    uart_printf("[*] ARM memory size: %x\r\n", arm_mem.size);
}

void _reboot(void){
    uart_printf("Rebooting...\r\n\r\n");
    // delay(10000);
    reset(10);
    // while(1);
}

void _echo(char *shell_buf){
    uart_printf(shell_buf);
}

void _ls(uint64 _initramfs_addr){
    cpio_ls((char*)_initramfs_addr);
}

void _cat(uint64 _initramfs_addr, char *filename){
    cpio_cat((char*)_initramfs_addr, filename);
}

void _parsedtb(char *fdt_base){
    dtb_traverse(fdt_base);
}

void *_malloc(char *size){
    int int_size = atoi(size);
    if(int_size==-1){
        uart_printf("[x] malloc size type is wrong or too large!\r\n");
        return NULL;
    }
    uart_printf("Ready to allocate %d size of memory!\r\n",int_size);
    return kmalloc(int_size);
}

void _exec(uint64 _initramfs_addr,char *filename){
   sched_new_user_prog((char *)_initramfs_addr, filename);
}

void _chmod_uart(){
    int mode = uart_switch_mode();
    if(mode==0)
        uart_printf("[*] Use synchronous uart now!\r\n");
    else
        uart_printf("[*] Use asynchronous uart now!\r\n");
}

int _setTimeout(char *shell_buf){
    char *msg, *tsec;
    msg = shell_buf;
    if(*msg != ' ')
        return -1;
    
    msg++;
    if(!*msg)
        return -1;
    
    tsec = msg;
    while(*(++tsec)){
        if(*tsec==' ')
            break;
    }
    if(*tsec != ' ')
        return -1;
    *tsec = 0;
    tsec++;
    if(!*tsec)
        return -1;
    
    int len;
    char *mem;

    len = strlen(msg) + 1;
    mem = kmalloc(len);
    memncpy(mem ,msg, len);
    timer_add_after((void(*)(void *))timeout_print, mem, atoi(tsec));
    return 0;
}

static void foo(){
    for (int i = 0; i < 10; ++i) {
        uart_printf("Thread id: %d %d\r\n", current->tid, i);
        delay(1000000);
        schedule();
    }
}

void _thread_test(){
    for (int i = 0; i < 3; ++i) {
        kthread_create(foo);
    }
}