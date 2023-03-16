#include "peripherals/mini_uart.h"
#include "mbox/mbox.h"
#include "power.h"
#include "utils.h"
#include "cpio/cpio.h"
#include "dtb/dtb.h"
#include "mem.h"


#define SHELL_BUFSIZE 64
#define REPLY_BUFSIZE 128
#define EXIT 255
#define NOT_FOUND 1


// char buf[SHELL_BUFSIZE];
const char hello_world[] = "Hello World!";
const char help_menu[] = "help   \t: print this help menu\r\nhello  \t: print Hello World!\r\ninfo   \t: print machine info\r\nreboot\t: reboot machine\r\nls    \t: list files\r\ncat   \t: show file content";

char *history[100];
int history_cnt = 0;

void split_comm_arg(char *buf, char **comm, char **arg) {
    *comm = buf;
    while(*buf != ' ') {
        buf += 1;
    }
    *buf = '\0';
    *arg = buf + 1;
    while(**arg <= ' ') {
        *arg += 1;
    }
}

int handle_input(char *buf) {
    // uart_send_string(buf);
    int tot_len = strlen(buf);

    // discard tailing space
    while(tot_len && buf[tot_len - 1] <= ' ') {
        buf[tot_len - 1] = '\0';
        tot_len -= 1;
    }
    // discard leading space
    while(tot_len && *buf <= ' ') {
        buf = buf + 1;
        tot_len -= 1;
    }
    char *comm;
    char *arg;
    split_comm_arg(buf, &comm, &arg);
    if(tot_len == 0) return 0;
    int cond = strncmp(comm, "hello", 5);
    if(cond == 0) {
        uart_send_string(hello_world);
        uart_send_string("\r\n");
        return 0;
    }

    cond = strncmp(comm, "help", 4);
    if(cond == 0) {
        uart_send_string(help_menu);
        uart_send_string("\r\n");
        return 0;
    }

    cond = strncmp(comm, "info", 4);

    if(cond == 0) {
        mbox_buf[0] = 7 * 4;
        mbox_buf[1] = 0;
        mbox_buf[2] = MBOX_GET_BOARD_REVISION;
        mbox_buf[3] = 4;
        mbox_buf[4] = 0;
        mbox_buf[5] = 0;
        mbox_buf[6] = 0;
        mbox_buf[7] = 0;
        if(mbox_call_func(MBOX_CH_PROPERTY_TAG)) {
            uart_send_string("BOARD REVISION: 0x");
            uart_send_u32(mbox_buf[5]);
            uart_send_string("\r\n");
        } else {
            uart_send_string("Error when gather info!!\r\n");            
        }
        mbox_buf[0] = 8 * 4;
        mbox_buf[1] = 0;
        mbox_buf[2] = MBOX_GET_ARM_MEMORY;
        mbox_buf[3] = 8;
        mbox_buf[4] = 0;
        mbox_buf[5] = 0;
        mbox_buf[6] = 0;
        mbox_buf[7] = 0;
        if(mbox_call_func(MBOX_CH_PROPERTY_TAG)) {
            uart_send_string("Base Address(in bytes): 0x");
            uart_send_u32(mbox_buf[5]);
            uart_send_string("\r\n");
            uart_send_string("Size(in bytes): 0x");
            uart_send_u32(mbox_buf[6]);
            uart_send_string("\r\n");
        } else {
            uart_send_string("Error when gather info!!\r\n");            
        }
        return 0;
    }

    cond = strncmp(comm, "reboot", 6);
    if(cond == 0) {
        uart_send_string("Reboot in 100 ticks\r\n");
        reset(100);
        return EXIT;
    }
    cond = strncmp(comm, "ls", 2);
    if(cond == 0) {
        list_files();
        return 0;
    }

    cond = strncmp(comm, "cat", 3);
    if(cond == 0) {
        cat_file(arg);
        return 0;
    }
    cond = strncmp(comm, "history", 3);
    if(cond == 0) {
        int x = history_cnt;
        if(x > 100) {
            x = 100;
        }
        for(int i = 0; i < x; i ++) {
            uart_send_string(history[i]);
            uart_send_string("\r\n");
        }
        return 0;
    }
    uart_send_string("Command not Found!\r\n");
    return NOT_FOUND;
}


const char shell_beg[] = "______________________\r\n"
"< 109550062 OSDI Shell >\r\n"
"----------------------\r\n"
"        \\   ^__^\r\n"
"         \\  (oo)\\_______\r\n"
"            (__)\\       )\\/\\\r\n"
"                ||----w |\r\n"
"                ||     ||\r\n";


void get_initramfs_addr(char *name, char *prop_name, char *data) {
    if(strncmp(name, "chosen", 6) == 0) {
        if(strncmp(prop_name, "linux,initrd-start", 18) == 0) {
            set_initramfs_addr(ntohi(*(unsigned int*)(data)));
        }
    }
}

void kernel_main(void* dtb_addr) {
    fdt_param_init(dtb_addr);
    fdt_traverse(&get_initramfs_addr);
    uart_init();
    uart_send_string(shell_beg);
    uart_send_string("# ");

	// while (1) {
	// 	uart_send(uart_recv());
	// }

    char cur;

    char *buf = simple_malloc(64);

    int idx = 0;
    memset(buf, 0, 64);
    while(1) {
        cur = uart_recv();
        // uart_send(cur - 12);
        if(cur == 127 || cur == 8) {
            if(idx == 0) continue;
            idx --;
            buf[idx] = '\0';
            uart_send(8);
            uart_send(127);
            continue;
        }
        if(cur == 13) {
            // uart_send_string("Enter Inputed:(");
            uart_send_string("\r\n");
            buf[idx] = '\0';
            int code = handle_input(buf);
            if(code == EXIT) {
                return;
            }
            history[history_cnt++ % 100] = buf;
            buf = simple_malloc(64);
            idx = 0;
            memset(buf, 0, 64);
            uart_send_string("# ");
            continue;
        }
        buf[idx++] = cur;
        if(idx == 63) idx = 0;
        uart_send(cur);
    }
}
