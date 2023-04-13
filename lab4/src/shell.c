#include "mini_uart.h"
#include "peripherals/mailbox.h"
#include "utils.h"
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
	char *async = "async";
	char *buddy = "buddy";
	char *dyn = "dyn";
    unsigned int *dest;
    // DEBUG
    // cmd = "ls";
    if (str_comp(cmd, hello)) {uart_puts("Hello World!\n");}
    else if (str_comp(cmd, help)) {
        uart_puts("help:\t\tprint this help menu\n");
        uart_puts("hello:\t\tprint Hello World!\n");
        uart_puts("mailbox:\tMailbox address and size\n");
        uart_puts("ls:\t\tshow the directory\n");
        uart_puts("cat:\\ttshow file content\n");
        uart_puts("lshw:\t\tshow hardware resuorces\n");
        uart_puts("reboot:\t\treboot the device\n");
        uart_puts("prog:\t\trun a user program\n");
		uart_puts("async_io:\tStarts a Read by Interrupt\n");
		uart_puts("buddy: Demo Buddy System[1, 2, 1, 1, 3]\n");
		uart_puts("dyn: Demo Dynamic Allocator (size: 2000)\n");
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
		exec_prog(dest);
    }
    else if (str_comp(cmd, async)){
		uart_puts("Asynchronous IO Starts, Please Type:");
		uart_puts("\n$ ");
		
		// testing, if success interrupt, then this will halt
		buf_clear(cmd, MAX_CMD);
		enable_mini_uart_interrupt();
		uart_async_getc(cmd, 16);
		delay(1);
		uart_async_send(cmd, 16);
		return 1;
	}
	else if (str_comp(cmd, buddy)){
		buddy_init();
	}
	else if (str_comp(cmd, dyn)){
		dyn_init();
	}
    else uart_puts("shell: command not found\n");
    buf_clear(cmd, MAX_CMD);
	return 0;
}

int str_comp(char *x, char *y){
    for (int i = 0; x[i] != '\0'; i++) {
        if(x[i]!=y[i])
            return 0;
	}
    return 1;
}

void exception_entry() {
    unsigned long spsrel1, elrel1, esrel1;
    asm volatile ("mrs %0, SPSR_EL1" : "=r" (spsrel1));
    uart_puts("SPSR_EL1: 0x");
    uart_hexlong(spsrel1);
    uart_puts("\n");
    asm volatile ("mrs %0, ELR_EL1" : "=r" (elrel1));
    uart_puts("ELR_EL1: 0x");
    uart_hexlong(elrel1);
    uart_puts("\n");
    asm volatile ("mrs %0, ESR_EL1" : "=r" (esrel1));
    uart_puts("ESR_EL1: 0x");
    uart_hexlong(esrel1);
    uart_puts("\n");
}

void print_core_timer(unsigned long frq, unsigned long cnt) {
	uart_puts("Core timer: ");
	uart_ulong(cnt/frq);
	uart_puts("\n");
}

void exec_prog(char * addr){
    uart_hex(addr);
	uart_puts("\n");
    asm volatile ("mov x0, 0");
    asm volatile ("msr spsr_el1, x0"); //Holds the saved process state when an exception is taken to EL1
    asm volatile ("msr elr_el1, %0": :"r" (addr));
    asm volatile ("mov x0, 0x60000");
    asm volatile ("msr sp_el0, x0");
    asm volatile ("eret");
}
