#include "mini_uart.h"
#include "peripherals/mailbox.h"
#include "cpio.h"
#include "dtb.h"
#define MAX_CMD 64

enum cmd_task {
    help,
    hello,
    reboot
};


void shell_input(char *cmd){
    int idx = 0;
    char c;
    while ((c = uart_getc()) != '\n')
    {
        /*TODO BS*/
        // if(c == 8 && idx > 0){
        //     cmd[--idx] = '\0';
        //     uart_send('\b');
        //     uart_send('\b');
        // }
        // else {
            uart_send(c);
            cmd[idx] = c;
            idx++;
        // }
    }
    uart_puts("\n");
}

/**
 * Define all commands
 */
unsigned int parse_cmd(char *cmd, void *dtb){
    char *help = "help";
    char *hello = "hello";
    char *mbx = "mailbox";
    char *reboot = "reboot";
    char *ls = "ls";
    char *cat = "cat";
    char *lshw = "lshw";
    char *initramfs = "initramfs";
    char *prog = "prog";
    unsigned int *dest;
    // DEBUG
    // cmd = "ls";
    if (str_comp(cmd, hello)) {uart_puts("Hello World!\n");}
    else if (str_comp(cmd, help)) {
        uart_puts("help:\tprint this help menu\n");
        uart_puts("hello:\tprint Hello World!\n");
        uart_puts("mailbox:\tMailbox address and size\n");
        uart_puts("ls:\tshow the directory\n");
        uart_puts("cat:\tshow file content\n");
        uart_puts("lshw:\tshow hardware resuorces\n");
        uart_puts("reboot:\treboot the device\n");
        uart_puts("prog:\trun a user program");
    }
    else if (str_comp(cmd, mbx)){
        mbox_call(MBOX_CH_PROP);
    }
    else if (str_comp(cmd, reboot)){
        reset(10);
    }
    else if (str_comp(cmd, ls)){
        initrd_list();
    }
    else if (str_comp(cmd, cat)){
        cat_list();
    }
    else if (str_comp(cmd, lshw)){
        dtb_list(dtb);
    }
    else if (str_comp(cmd, initramfs)){
        print_initramfs();
    }
    else if (str_comp(cmd, prog)){
        dest = load_prog("usr.img");
        uart_puts("\nProgram is loaded into address: ");
        uart_hex(dest);
        uart_puts("\n");
        exec_prog(dest);
    }
    else uart_puts("shell: command not found\n");
    buf_clear(cmd);
}

int str_comp(char *x, char *y){
    for (int i = 0; x[i] != '\0'; i++) {
        if(x[i]!=y[i])
            return 0;
	}
    return 1;
}

void buf_clear(char *buf){
    for(int i = 0; i < MAX_CMD; i++){
        buf[i] = '\0';
    }
}

void exception_entry() {
    // unsigned long spsrel1, elrel1, esrel1;
    // asm volatile ("mrs %0, SPSR_EL1" : "=r" (spsrel1));
    // uart_puts("SPSR_EL1: 0x");
    // uart_hexlong(spsrel1);
    // uart_puts("\n");
    // asm volatile ("mrs %0, ELR_EL1" : "=r" (elrel1));
    // uart_puts("ELR_EL1: 0x");
    // uart_hexlong(elrel1);
    // uart_puts("\n");
    // asm volatile ("mrs %0, ESR_EL1" : "=r" (esrel1));
    // uart_puts("ESR_EL1: 0x");
    // uart_hexlong(esrel1);
    uart_puts("\n");
}

void exec_prog(char * addr){
    uart_hex(addr);
    asm volatile ("mov x0, 0x3c0");
    asm volatile ("msr spsr_el1, x0"); //Holds the saved process state when an exception is taken to EL1
    asm volatile ("msr elr_el1, %0": :"r" (addr));
    asm volatile ("mov x0, 0x100000");
    asm volatile ("msr sp_el0, x0");
    asm volatile ("eret");
}