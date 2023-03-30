#include "peripherals/mini_uart.h"
#include "mbox/mbox.h"
#include "power.h"
#include "utils.h"
#include "cpio/cpio.h"
#include "dtb/dtb.h"
#include "mem.h"
#include "time.h"
#include "interrupt.h"
#include "ds/heap.h"
#include "demo.h"


#define SHELL_BUFSIZE 64
#define REPLY_BUFSIZE 128
#define EXIT 255
#define NOT_FOUND 1

#define ARRAY_SIZE(arr, type) (sizeof(arr) / sizeof(type))

const char hello_world[] = "Hello World!";
const char help_menu[] = "help   \t: print this help menu\r\nhello  \t: print Hello World!\r\ninfo   \t: print machine info\r\nreboot\t: reboot machine\r\nls    \t: list files\r\ncat   \t: show file content";
// char buf[SHELL_BUFSIZE];

struct k_timeout msg;
char *history[10];
int history_cnt = 0;
struct CmdNode {
    char *comm;
    unsigned int (*cb)(char *args);
};

struct ds_heap heap;

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
unsigned int _hello_cb(char *args) {
    uart_send_string(hello_world);
    uart_send_string("\r\n");
    return 0;
}

unsigned int _help_cb(char *args) {
    uart_send_string(help_menu);
    uart_send_string("\r\n");
    return 0;
}

unsigned int _setTimeout_cb(char *arg) {
    char *buf;
    char *num;
    split_comm_arg(arg, &buf, &num);
    int t = 0;
    for(char *ch = num; *ch != '\0'; ch++) {
        t = t * 10 + *ch - '0';
    }
    k_timeout_submit(&msg, arg, t);
    return 0;
}

unsigned int _test_heap(char *arg) {
    unsigned int x = arg[0] - '0';
    if(x == 0) {
        while(heap.size) {
            uart_send_dec(*(unsigned int*)ds_heap_front(&heap));
            uart_send_string("\r\n");
            ds_heap_pop(&heap);
        }
        return 0;
    }
    ds_heap_push(&heap, &x, sizeof(unsigned int), x);
    return 0;
}
unsigned int _info_cb(char *arg) {
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

unsigned int _reboot_cb(char *arg) {
    uart_send_string("Reboot in 100 ticks\r\n");
    reset(100);
    return EXIT;
}

unsigned int _ls_cb(char *arg) {
    list_files();
    return 0;
}

unsigned int _cat_cb(char *arg) {
    cat_file(arg);
    return 0;
}

unsigned int _load_cb(char *arg) {
    load_program(arg);
    return 0;
}

unsigned _history_cb(char *arg) {
    int x = history_cnt;
    if(x > 10) {
        x = 10;
    }
    for(int i = 0; i < x; i ++) {
        uart_send_string(history[i]);
        uart_send_string("\r\n");
    }
    return 0;
}

unsigned int _test_uart_cb(char *arg) {
    enable_aux_interrupt();
    char buf[32];
    memset(buf, 0, 32);
    char c;
    int i = 0;
    while(1) {
        c = async_uart_recv();
        if(c == 255) continue;
        if(c == '\r') {
            async_uart_send('\r');
            async_uart_send('\n');
            break;
        }
        async_uart_send(c);
        buf[i++] = c;
    }
    return 0;
}
struct CmdNode cmds[] = {
    {
        .comm = "hello",
        .cb = _hello_cb
    },
    {
        .comm = "help",
        .cb = _help_cb
    }, 
    {
        .comm = "info",
        .cb = _info_cb
    },
    {
        .comm = "reboot",
        .cb = _reboot_cb
    },
    {
        .comm = "ls",
        .cb = _ls_cb
    }, 
    {
        .comm = "cat",
        .cb = _cat_cb
    },
    {
        .comm = "load",
        .cb = _load_cb
    },
    {
        .comm = "history",
        .cb = _history_cb
    },
    {
        .comm = "test_uart",
        .cb = _test_uart_cb
    },
    {
        .comm = "test_heap",
        .cb = _test_heap
    },
    {
        .comm = "setTimeout",
        .cb = _setTimeout_cb
    }
};




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
    int cond;
    int sz = ARRAY_SIZE(cmds, struct CmdNode);
    for(int i = 0; i < sz; i ++) {
        cond = strncmp(cmds[i].comm, comm, 64);
        if(cond == 0) {
            return cmds[i].cb(arg);
        }
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

void print_msg(void *arg) {
    uart_send_string(arg);
}

void kernel_main(void* dtb_addr) {
    // uart_send_string("fucked\r\n");
    fdt_param_init(dtb_addr);
    fdt_traverse(&get_initramfs_addr);
    k_event_queue_init();
    demo_init();
    uart_init();
    enable_interrupt();
    enable_aux_interrupt();
    k_timeout_init(&msg, &print_msg);
    uart_send_string(shell_beg);
    uart_send_string("# ");


    char cur;

    char *buf = simple_malloc(256);

    int idx = 0;
    memset(buf, 0, 64);
    while(1) {
        cur = uart_recv();
        if(cur == 255) continue;
        if(cur == 127 || cur == 8) {
            if(idx == 0) continue;
            idx --;
            buf[idx] = '\0';
            uart_send(8);
            uart_send(127);
            continue;
        }
        if(cur == 13) {
            uart_send_string("\r\n");
            buf[idx] = '\0';
            int code = handle_input(buf);
            if(code == EXIT) {
                return;
            }
            // history[history_cnt++ % 100] = buf;
            // buf = simple_malloc(256);
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
