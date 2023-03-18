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
    // DEBUG
    // cmd = "ls";
    if (str_comp(cmd, hello)) {uart_puts("Hello World!\n");}
    else if (str_comp(cmd, help)) {
        uart_puts("help\t: print this help menu\n");
        uart_puts("hello\t: print Hello World!\n");
        uart_puts("mailbox\t: Mailbox address and size\n");
        uart_puts("ls\t: show the directory\n");
        uart_puts("cat\t: show file content\n");
        uart_puts("lshw\t: show hardware resuorces\n");
        uart_puts("reboot\t: reboot the device\n");
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
