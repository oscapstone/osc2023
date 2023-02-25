#include "command.h"
#include "terminal.h"
#include "mailbox.h"
#include "uart.h"

struct command commands[] = { 
        {
                .name = "help",
                .help = "Show help message!\n",
                .func = help,
        },
        {   
                .name = "lshw",
                .help = "Show some HW informations\n",
                .func = lshw,
        },  
        {   
                .name = "hello",
                .help = "Print \'hello world\'\n",
                .func = hello,
        },  
        {   
                .name = "reboot",
                .help = "Reboot the device",
                .func = reboot,
        },  
	
	// ALWAYS The last item of the array!!!
        {   
                .name = "NULL",         // The end of the array
        }   
};

int help(){
        uart_puts(
                        "help\t: print this help message.\n"
                        "hello\t: print Hello World!\n"
                        "lshw\t: print the information of device \n"
                        "reboot\t: reboot the device.\n"
                 );
        return 0;
}

int hello(){
        uart_puts("Hello World!\n");
        return 0;
}

int invalid_command(const char* s){
        uart_putc('`');
        uart_puts(s);
        uart_putc('`');
        uart_puts(" is invalid command! Please use `help` to list commands\n");        return 0;
}

int lshw(void){
        uart_puts("Board version\t: ");
        mbox[0] = 7 * 4;
        mbox[1] = MAILBOX_REQ;
        mbox[2] = TAG_BOARD_VER;
        mbox[3] = 4;
        mbox[4] = 0;
        mbox[5] = 0;
        mbox[6] = TAG_LAST;

        if(mailbox_config(CHANNEL_PT)){
                uart_puth(mbox[5]);
        }
        uart_puts("\nDevice base Mem Addr\t: ");
        mbox[0] = 8 * 4;
        mbox[1] = MAILBOX_REQ;
        mbox[2] = TAG_ARM_MEM;
        mbox[3] = 8;
        mbox[4] = 0;
        mbox[5] = 0;
        mbox[6] = 0;
        mbox[7] = TAG_LAST;
        if(mailbox_config(CHANNEL_PT)){
                uart_puth(mbox[5]);
                uart_puts("\nDevice Mem size\t: ");
                uart_puth(mbox[6]);
        }
        uart_putc('\n');
        return 0;
}

int reboot(){
        *PM_RSTC = PM_PASSWORD | 0x20;          // Reset
        *PM_WDOG = PM_PASSWORD | 180;
        return 0;
}
