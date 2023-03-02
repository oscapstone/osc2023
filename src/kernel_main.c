#include "peripherals/mini_uart.h"
#include "mbox/mbox.h"
#include "power.h"


#define SHELL_BUFSIZE 64
#define REPLY_BUFSIZE 128
#define EXIT 255
#define NOT_FOUND 1


char buf[SHELL_BUFSIZE];
const char hello_world[] = "Hello World!";
const char help_menu[] = "help   \t: print this help menu\r\nhello  \t: print Hello World!\r\ninfo   \t: print machine info\r\nreboot\t: reboot machine";

int strncmp(char *s1, char *s2, unsigned int maxlen) {
    int i;
    for(i = 0; i < maxlen && s1[i] != '\0' && s2[i] != '\0'; i ++) {
        if(s1[i] < s2[i]) {
            return -1;
        }
        if(s1[i] > s2[i]) {
            return 1;
        }
    }
    if(s1[i] == '\0' && s2[i] != '\0') {
        return -1;
    }
    if(s1[i] != '\0' && s2[i] == '\0') {
        return 1;
    }
    return 0;
}

int strlen(const void *buf) {
    int ret = 0;
    for(const char *ch = (const char *)buf; *ch != '\0'; ch ++) {
        ret += 1;
    }
    return ret;
}

int handle_input(char *buf) {
    // uart_send_string(buf);
    int tot_len = strlen(buf);
    // discard leading space
    while(tot_len && *buf == ' ') {
        buf = buf + 1;
        tot_len -= 1;
    }
    // discard tailing space
    while(buf[tot_len - 1] == ' ') {
        buf[tot_len - 1] = '\0';
        tot_len -= 1;
    }
    if(tot_len == 0) return 0;
    int cond = strncmp(buf, "hello", 5);
    if(cond == 0) {
        uart_send_string(hello_world);
        uart_send_string("\r\n");
        return 0;
    }

    cond = strncmp(buf, "help", 4);
    if(cond == 0) {
        uart_send_string(help_menu);
        uart_send_string("\r\n");
        return 0;
    }

    cond = strncmp(buf, "info", 4);

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

    cond = strncmp(buf, "reboot", 6);
    if(cond == 0) {
        uart_send_string("Reboot in 100 ticks\r\n");
        reset(100);
        return EXIT;
    }

    uart_send_string("Command not Found!\r\n");
    return NOT_FOUND;
}

void memset(void *dest, char val, unsigned int size) {
    for(int i = 0; i < size; i ++) {
        *(char*)(dest + i) = val;
    }
}

void kernel_main(void) {
    uart_init();
    uart_send_string("Hello, World!\r\n# ");

	// while (1) {
	// 	uart_send(uart_recv());
	// }

    char cur;

    int idx = 0;
    memset(buf, 0, 64);
    while(1) {
        cur = uart_recv();
        // uart_send(cur - 12);
        if(cur == 127) {
            if(idx == 0) continue;
            idx --;
            buf[idx] = '\0';
            // uart_send(8);
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
