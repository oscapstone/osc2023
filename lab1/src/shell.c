#include "uart.h"
// #include "util.h"
#include "mbox.h"
#include "reboot.h"

int str_cmp(char *s1, char *s2){
    int i = 0;
    if (s1[0] == '\0' && s2[0] == '\0') return 1;
    while(s1[i]){   
        if (s1[i] != s2[i]) return 0;
        i++;
    }
    return 1;
}
void shell_init(){
	uart_init();
    uart_flush();
    uart_printf("\nWelcome!\n");
}
void shell_input(char *cmd){
    uart_printf("\t#");
    cmd[0] = '\0';
    char c;
    char co[2];
    int c_end = 0;
    while( (c = uart_read()) != '\n'){
        co[0] = c;
        co[1] = '\0';
        uart_printf(co);
        cmd[c_end] = c;
        c_end ++;
        cmd[c_end] = '\0';
    }
    uart_printf("\n");
}
void shell_controller(char *cmd){    
    if (str_cmp(cmd, "")){
        return;
    }
    else if (str_cmp(cmd, "help")){
        uart_printf("help\t: print the help menu\n");
        uart_printf("hello\t: print Hello World!\n");
        uart_printf("mailbox\t: get the hardware's info!\n");
        uart_printf("reboot\t: reboot the device\n");
    }
    else if (str_cmp(cmd, "hello")){
        uart_printf("Hello World!\n");
    }
    else if (str_cmp(cmd, "mailbox")){
        mbox_board_revision();
        mbox_memory_info();
    }
    else if (str_cmp(cmd, "reboot")){
        uart_printf("Rebooting..\n");
        reset(100);
        while(1);
    }    
    else{
        uart_printf("Commont not found!\n");
    }   
    
}
