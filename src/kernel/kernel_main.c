#include "peripherals/mini_uart.h"
#include "mbox/mbox.h"
#include "power.h"
#include "utils.h"
#include "ds/list.h"
#include "cpio/cpio.h"
#include "dtb/dtb.h"
#include "mem/mem.h"
#include "time.h"
#include "interrupt.h"
#include "ds/heap.h"
#include "test/demo.h"
#include "mem/page.h"
#include "process.h"
#include "thread.h"
#include "mmu/mmu.h"
#include "fs/tmpfs.h"
#include "fs/vfs.h"
#include "framebuffer.h"
#include "sd.h"
#include "fs/fat32.h"

extern struct Trapframe_t *gen_trapframe();
extern struct ds_list_head process_list;

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
    k_timeout_submit(&msg, buf, t);
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
    // list_files();
    struct Process_t *cur_proc = process_get_current();
    vfs_apply_dir(cur_proc->cur_vnode, &tmpfs_print_name);
    return 0;
}

unsigned int _cat_cb(char *arg) {
    // cat_file(arg);

    struct file *f;
    // uart_send_string(arg);
    int err = vfs_open(arg, O_READ, &f);

    char _buf[100];
    int n;
    if(err == 0) {
        while(n = vfs_read(f, _buf, 100)) {
            uart_send_n(_buf, n);
        }
        vfs_close(f);
    } else {
        uart_send_dec(-err);
        uart_send_string("Error when opening the file\r\n");
        return err;
    }
}

unsigned int _load_cb(char *arg) {
    load_program(arg);
    // process_load(arg);
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
void _test_thread_get_current() {
    return;
    // uart_send_u64(thread_get_current());
}
void _test_thread_set_current() {
    asm volatile(
        "mov x0, 1\n"
        "msr tpidr_el1, x0\n"
    );
}

void foo() {
    for(int i = 0; i < 10; i ++) {
        uart_send_string("Thread id: ");
        uart_send_dec(thread_get_current());
        uart_send_string(" ");
        uart_send_dec(i);
        uart_send_string("\r\n");
        delay(1000000);
        schedule(0);
    }
    return;
}

static void _test_thread() {
    int N = 3;
    for(int i = 0; i < N; ++i) { // N should > 2
        thread_create(foo, NULL);
    }
    schedule(0);
}

static void _test_fork() {
    // uint64_t n = process_fork();
    // if(n == 0) {
    //     while(1) {
    //         asm volatile("nop");
    //     }
    // }
    asm volatile(
        "mov x8, 4\n"
        "svc 0\n"
    );
    uint64_t pid;
    asm volatile(
        "mov %[pid], x0":[pid]"=r"(pid)
    );
    if(pid == 0) {
        uart_send_string("after fork\r\n");
        while(1) {
            asm volatile("nop");
        }
    }
    // uart_send_string("What happen\r\n");
    // while(1){
    //     schedule(0);
    // }
}
unsigned int _touch(char *arg) {
    // cat_file(arg);
    // struct file *f;
    struct vnode* target;
    int err = vfs_create(arg, O_CREAT, &target);

    // int err = vfs_open(arg, O_CREAT, &f);
    // err = vfs_close(f);
    // uart_send_dec(err);
    return 0;
}
unsigned int _open(char *arg) {
    struct file *f;
    int err = vfs_open(arg, O_READ, &f);
    uart_send_dec(err);
    return 0;
}
unsigned int _test_write(char *arg) {
    struct file *f;
    int err = vfs_open(arg, O_WRITE, &f);
    char _buf[] = "TETSTING WRITE\r\n";
    if(err == 0) {
        for(int i = 0; i < 10; i ++) {
            err = vfs_write(f, _buf, strlen(_buf));
            if(err < 0) {
                uart_send_string("Error when writing\r\n");
            }
        }
        err = vfs_close(f);
        if(err < 0) {
            uart_send_string("Error when closing\r\n");
        }
    } else {
        uart_send_string("Error when opening file\r\n");
    }
}
unsigned int _test_read(char *arg) {
    struct file *f;
    int err = vfs_open(arg, O_READ, &f);
    char _buf[10];
    if(err == 0) {
        int n = vfs_read(f, _buf, 5);
        while(n > 0) {
            uart_send_n(_buf, n);
            n = vfs_read(f, _buf, 5);
        }
        err = vfs_close(f);
        if(err < 0) {
            uart_send_string("Error when closing\r\n");
        }
    } else {
        uart_send_string("Error when opening file\r\n");
    }
}
unsigned int _mkdir(char *arg) {
    int err = vfs_mkdir(arg);
    if(err) {
        uart_send_string("Error happen when creating dir\r\n");
    }
    return 0;
}

unsigned int _cd(char *arg) {
    struct vnode *dir;
    int err = vfs_lookup(arg, &dir);

    if(err != 0) {
        uart_send_string("Error when switching directory\r\n");
    } else {
        struct Process_t *proc = process_get_current();
        proc->cur_vnode = dir;
        proc->mnt = dir->mount;
    }
}
unsigned int _mount(char *arg) {
    char *mntpoint, *fsname;
    split_comm_arg(arg, &mntpoint, &fsname);
    // vfs_mkdir(mntpoint);
    vfs_mount(mntpoint, fsname);
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
    },
    {
        .comm = "demo_page",
        .cb = demo_page
    },
    {
        .comm = "test_simple_alloc",
        .cb = test_simple_alloc
    },
    {
        .comm = "test_random",
        .cb = test_random
    },
    {
        .comm = "test_kmalloc",
        .cb = test_kmalloc
    },
    {
        .comm = "get_t",
        .cb = _test_thread_get_current
    }, {
        .comm = "set_t",
        .cb = _test_thread_set_current
    },
    {
        .comm = "sched",
        .cb = schedule
    },
    {
        .comm = "test_thread",
        .cb = _test_thread
    },
    {
        .comm = "test_fork",
        .cb = _test_fork
    },
    {
        .comm = "touch",
        .cb = _touch
    },
    {
        .comm = "test_write",
        .cb = _test_write
    },
    {
        .comm = "test_read",
        .cb = _test_read
    },
    {
        .comm = "open",
        .cb = _open
    },
    {
        .comm = "mkdir",
        .cb = _mkdir
    },
    {
        .comm = "cd",
        .cb = _cd
    },
    {
        .comm = "mount",
        .cb = _mount
    },
    {
        .comm = "sync",
        .cb = fat32_sync
    }
    // {
    //     .comm = "close",
    //     .cb = _close
    // }
};

int handle_input(char *buf) {
    // uart_send_string(buf);
    int tot_len = strlen(buf);

    if(tot_len == 0) {
        return 0;
    }
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

void shell() {
    // uart_send_string("Inside SHELL\r\n");
    char cur;

    char *buf = simple_malloc(256);

    int idx = 0;
    memset(buf, 0, 64);
    while(1) {
        // cur = uart_recv();
        uint32_t n = async_uart_rx_buf(&cur, 1);
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
        // schedule();
    }
}

extern struct Process_t init_process;
void go_el0() {
    // uint64_t x, y, z;
    // asm volatile(
    //     // "mrs x10, sp_el0\n"
    //     "mov %0, lr\n":"=r"(x)
    // );
    // uart_send_u64(x);
    // uart_send_string("\r\n");
    asm volatile(
        "mov x10, 0x340\n"
        "msr spsr_el1, x10\n"
        "mov x10, lr\n"
        "msr elr_el1, x10\n"
        "eret\n"
    );
}
void kernel_main(void* dtb_addr) {
    // uart_send_string("fucked\r\n");
    smem_init();
    fdt_param_init(dtb_addr);
    k_event_queue_init();
    demo_init();
    uart_init();
    k_timeout_init(&msg, &print_msg);
    kmalloc_init();
    memory_reserve(0 + KERNEL_SPACE_OFFSET, 0x15000000 + KERNEL_SPACE_OFFSET);
    memory_reserve(0x20000000 + KERNEL_SPACE_OFFSET, 0x21000000 + KERNEL_SPACE_OFFSET);
    memory_reserve(0x2f000000 + KERNEL_SPACE_OFFSET, 0x2f100000 + KERNEL_SPACE_OFFSET);
    memory_reserve(0x3a000000 + KERNEL_SPACE_OFFSET, 0x3c000000 + KERNEL_SPACE_OFFSET);
    fdt_traverse(&get_initramfs_addr);
    fdt_traverse(&set_init_mem_region);
    enable_interrupt();
    uart_switch_func(UART_DEFAULT);
    enable_aux_interrupt();
    vfs_init();
    init_fs();
    uart_switch_func(UART_ASYNC);
    mount_initramfs();
    thread_control_init();
    process_control_init();
    uart_send_string(shell_beg);
    uart_send_string("# ");

    vfs_mkdir("/dev");
    vfs_mkdir("/tmp");
    setup_dev_uart();
    setup_dev_framebuffer();

    sd_init();


    setup_boot();

    struct Process_t *shell_proc = create_process_instance();
    shell_proc->ttbr0_el1 = 0x1000;
    memcpy(shell_proc->name, "shell", 6);

    struct Thread_t *th = process_thread_create(shell, NULL, shell_proc, 1);

    // struct ds_list_head *front = ds_list_front(&(process_list));
    // while(front != &(process_list)) {
    //     struct Process_t *proc = container_of(front, struct Process_t, pl_head);
    //     front = ds_list_front(front);
    // }
    schedule_init();
}
