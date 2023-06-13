#include "syscall.h"
#include "thread.h"
#include "uart.h"
#include "utils.h"
#include "initramfs.h"
#include "mm.h"
#include "mailbox.h"
#include "vfs.h"

// #define SYSCALL_DBG 1

extern void print_tree();

void syscall_getpid(struct trap_frame *tf)
{
    task_t *current = get_current_thread();
    tf->gprs[0] = current->tid;
}
void syscall_uart_read(struct trap_frame *tf)
{
    if (!UART_READABLE()) schedule();
    char *buf = tf->gprs[0];
    size_t size = tf->gprs[1];
    size_t i;
    for (i = 0; i < size; i++)
        buf[i] = _kuart_read();
    tf->gprs[0] = i;
}
void syscall_uart_write(struct trap_frame *tf)
{
    char *buf = tf->gprs[0];
    size_t size = tf->gprs[1];
    size_t i;
    for (i = 0; i < size; i++)
        _kuart_write(buf[i]);
    tf->gprs[0] = i;
}

void run_user_prog(char *user_text)
{
    task_t *current = get_current_thread();
    mappages(current->pgd, USER_STK_LOW, STACKSIZE, 0);
    // _run_user_prog(user_text, exit, STACK_BASE(current->user_stack, current->user_stack_size));
    _run_user_prog(user_text, exit, USER_STK_HIGH);
}

void _run_user_prog(char *user_text, char *callback, char *stack_base)
{
    //write lr
    __asm__ __volatile__("mov x30, %[value]"
                         :
                         : [value] "r" (callback)
                         : "x30");
    //write fp
    __asm__ __volatile__("mov x29, %[value]"
                         :
                         : [value] "r" (stack_base)
                         : "x29");
    //allow interrupt
    write_sysreg(spsr_el1, 0);
    write_sysreg(elr_el1, user_text);
    write_sysreg(sp_el0, stack_base);
    asm volatile("eret");
}

//Run the program with parameters.
//The exec() functions return only if an error has occurred.
//The return value is -1
void syscall_exec(struct trap_frame *tf)
{
    const char *name = tf->gprs[0];
    char **const argv = tf->gprs[1];
    char *real_argv[2] = {name, NULL};
    //reform all syscall_function into proper form
    size_t file_size;
    char *content = _initramfs.file_content(&_initramfs, name, &file_size);
    if (content == NULL) {
        //file not found or not a valid new ascii format cpio archive file
        tf->gprs[0] = -1;
    } else {
        // run_user_prog(content);
        tf->gprs[0] = _initramfs.exec(&_initramfs, real_argv);
        //never go here
    }
}
void syscall_fork(struct trap_frame *tf)
{
    //if is parent thread, retrurn child thread id
    //otherwise, return 0
    task_t *current = get_current_thread();
    //compile add following instruction truncating 0xffffxxxxxxxxxxxx to 0xxxxxxxxx
    //sxtw    x0, w0
    //task_t *child = copy_run_thread(current);
    //fast fix
    task_t *child = copy_run_thread(current) | KERN_BASE;
    tf->gprs[0] = child->tid;
}
void syscall_exit(struct trap_frame *tf)
{
    _exit(0);
}
void syscall_mbox_call(struct trap_frame *tf)
{
    unsigned char ch = tf->gprs[0];
    unsigned int *mbox = tf->gprs[1];
    unsigned int *kernel_addr = walk(get_current_thread()->pgd, mbox, 0);
    mailbox_call(kernel_addr, ch);
    tf->gprs[0] = kernel_addr[1];//mbox[1];
}
void syscall_kill(struct trap_frame *tf)
{
    int pid = tf->gprs[0];
    //TODO
    //You don’t need to implement this system call if you prefer to kill a process using the POSIX Signal stated in Advanced Exercise 1.
    task_t *t = tid2task[pid];
    disable_interrupt();
    if (t->exit_state | t->state == TASK_RUNNING) {
        list_del(t);
        list_add_tail(t, &stop_queue);
    }
    test_enable_interrupt();
    t->exit_code = 9;
    t->state = 0;
    t->exit_state = EXIT_ZOMBIE;
    // schedule();
}

//register a signal handler
void syscall_signal(struct trap_frame *tf)
{
    int signum = tf->gprs[0];
    signal_handler_t handler = tf->gprs[1];
    if (signum < 0 || signum > MAX_SIGNAL) {
        tf->gprs[0] = NULL;
        return;
    }
    task_t *current = get_current_thread();
    signal_handler_t prev_handler = current->reg_sig_handlers[signum];
    current->reg_sig_handlers[signum] = handler;
    tf->gprs[0] = prev_handler;
}

void _add_signal(pid_t recv_pid, int signum)
{
    task_t *target = tid2task[recv_pid];
    struct signal *new_sig = kmalloc(sizeof(struct signal));
    new_sig->handler_user_stack = alloc_pages(1);
    new_sig->handler_user_stack_size = PAGE_SIZE;
    new_sig->tf = kmalloc(sizeof(struct trap_frame));
    new_sig->signum = signum;
    new_sig->handling = 0;

    INIT_LIST_HEAD(&(new_sig->node));
    disable_interrupt();
    list_add_tail(&(new_sig->node), &(target->pending_signal_list));
    test_enable_interrupt();
}

void syscall_sigkill(struct trap_frame *tf)
{
    pid_t recv_pid = tf->gprs[0];
    int signum = tf->gprs[1];
    if (signum < 0 || signum > MAX_SIGNAL) {
        tf->gprs[0] = -1;
        return;
    }
    if (recv_pid < 0 || recv_pid >= MAX_TASK_CNT || tid2task[recv_pid] == NULL) {
        tf->gprs[0] = -1;
        return;
    }
    _add_signal(recv_pid, signum);
    tf->gprs[0] = 0;
}

void syscall_sigreturn(struct trap_frame *tf)
{
    task_t *current = get_current_thread();
    disable_interrupt();
    struct signal *handled = list_entry(current->pending_signal_list.next, struct signal, node);
    list_del(&(handled->node));
    test_enable_interrupt();
    memcpy(tf, handled->tf, sizeof(struct trap_frame));
    kfree(handled->tf);
    free_page(handled->handler_user_stack);
    kfree(handled);
    //do not modify the original trap frame so that user process don't aware of enter of signal
}
//TODO syscalls related to vfs

//int open(const char *pathname, int flags);
void syscall_open(struct trap_frame *tf)
{
    const char *pathname = tf->gprs[0];
    //The test program do not know FILE_READ, FILE_WRITE, and FILE_APPEND
    //ADD THESE FLAGS MANUALLY.
    int flags = tf->gprs[1] | FILE_READ | FILE_WRITE | FILE_APPEND;
    struct file *target;
    if (vfs_open(pathname, flags, &target)) {
        tf->gprs[0] = -1;
        return;
    }
    task_t *current = get_current_thread();
    int fd = (target - (current->open_files));
    tf->gprs[0] = fd;
#ifdef SYSCALL_DBG
    uart_write_string("open ");
    uart_write_string(pathname);
    uart_write_string(" with flags: ");
    uart_write_no_hex(flags);
    uart_write_string(" fd: ");
    // uart_write_no(((char *)target - (char *)(current->open_files)) / sizeof(struct file));
    uart_write_no(fd);
    uart_write_string("\n");
    print_tree();
#endif
}

//int close(int fd);
void syscall_close(struct trap_frame *tf)
{
    int fd = tf->gprs[0];
    task_t *current = get_current_thread();
    tf->gprs[0] = (fd >= 0 && fd <= MAX_FD) ? vfs_close(&(current->open_files[fd])) : -1;
#ifdef SYSCALL_DBG
    uart_write_string("close ");
    uart_write_no(fd);
    uart_write_string("\n");
    print_tree();
#endif
}

// remember to return read size or error code
// long write(int fd, const void *buf, unsigned long count);
void syscall_write(struct trap_frame *tf)
{
    disable_interrupt();
    int fd = tf->gprs[0];
    if (fd < 0 || fd > MAX_FD) {
        tf->gprs[0] = 0;
        test_enable_interrupt();
        return;
    }
    task_t *current = get_current_thread();
    struct file *f = &(current->open_files[fd]);
    const void *buf = tf->gprs[1];
    unsigned long count = tf->gprs[2];
    
    tf->gprs[0] = vfs_write(f, buf, count);
    for (int idle = 1000; idle; idle--);
    test_enable_interrupt();
#ifdef SYSCALL_DBG
    uart_write_string("write ");
    uart_write_no(fd);
    uart_write_string(" with ");
    uart_write_string((char *)buf);
    uart_write_string("\n");
    print_tree();
#endif
}

// remember to return read size or error code
// long read(int fd, void *buf, unsigned long count);
void syscall_read(struct trap_frame *tf)
{
    disable_interrupt();
    int fd = tf->gprs[0];
    if (fd < 0 || fd > MAX_FD) {
        tf->gprs[0] = 0;
        test_enable_interrupt();
        return;
    }
    task_t *current = get_current_thread();
    struct file *f = &(current->open_files[fd]);
    
    void *buf = tf->gprs[1];
    unsigned long count = tf->gprs[2];
    tf->gprs[0] = vfs_read(f, buf, count);
    test_enable_interrupt();
#ifdef SYSCALL_DBG
    uart_write_string("read ");
    uart_write_no(fd);
    uart_write_string(" with ");
    uart_write_string((char *)buf);
    uart_write_string("\n");
    print_tree();
#endif
}

// you can ignore mode, since there is no access control
// int mkdir(const char *pathname, unsigned mode);
void syscall_mkdir(struct trap_frame *tf)
{
    disable_interrupt();
    const char *pathname = tf->gprs[0];
    //this field will be ignored
    // unsigned mode = tf->gprs[1];
    tf->gprs[0] = vfs_mkdir(pathname);
    test_enable_interrupt();
#ifdef SYSCALL_DBG
    uart_write_string("mkdir ");
    uart_write_string(pathname);
    uart_write_string("\n");
    print_tree();
#endif
}

// you can ignore arguments other than target (where to mount) and filesystem (fs name)
// int mount(const char *src, const char *target, const char *filesystem, unsigned long flags, const void *data);
void syscall_mnt(struct trap_frame *tf)
{
    disable_interrupt();
    // const char *src = tf->gprs[0]; //ignored
    const char *target = tf->gprs[1];
    const char *filesystem = tf->gprs[2];
    // unsigned long flags = tf->gprs[3]; //ignored
    // const void *data = tf->gprs[4]; //ignored
    tf->gprs[0] = vfs_mount(target, filesystem);
    test_enable_interrupt();
#ifdef SYSCALL_DBG
    uart_write_string("mnt ");
    uart_write_string(target);
    uart_write_string(" ");
    uart_write_string(filesystem);
    uart_write_string("\n");
    print_tree();
#endif
}

// int chdir(const char *path);
void syscall_chdir(struct trap_frame *tf)
{
    disable_interrupt();
    const char *path = tf->gprs[0];
    tf->gprs[0] = vfs_chdir(path);
    test_enable_interrupt();
#ifdef SYSCALL_DBG
    uart_write_string("chdir ");
    uart_write_string(path);
    uart_write_string("\n");
    print_tree();
#endif
}

// syscall number : 18
// you only need to implement seek set
// long lseek64(int fd, long offset, int whence);
void syscall_lseek64(struct trap_frame *tf)
{
    disable_interrupt();
    int fd = tf->gprs[0];
    if (fd < 0 || fd > MAX_FD) {
        tf->gprs[0] = -1;
        test_enable_interrupt();
        return;
    }

    task_t *current = get_current_thread();
    struct file *f = &(current->open_files[fd]);

    long offset = tf->gprs[1];
    int whence = tf->gprs[2];
    tf->gprs[0] = f->f_ops->lseek64(f, offset, whence);

    test_enable_interrupt();
}

// syscall number : 19
// int ioctl(int fd, unsigned long request, ...);
//
// // ioctl 0 will be use to get info
// // there will be default value in info
// // if it works with default value, you can ignore this syscall
// ioctl(fb, 0, &fb_info)
// // remember to translate userspace address to kernel space

struct framebuffer_info {
  unsigned int width;
  unsigned int height;
  unsigned int pitch;
  unsigned int isrgb;
};

void syscall_ioctl(struct trap_frame *tf)
{
    disable_interrupt();
    int fd = tf->gprs[0];
    if (fd < 0 || fd > MAX_FD) {
        tf->gprs[0] = -1;
        return;
    }
    task_t *current = get_current_thread();
    struct file *f = &(current->open_files[fd]);
    unsigned long request = tf->gprs[1];
    if (request != 0) return;
    
    struct framebuffer_info *fb_info = tf->gprs[2];
    /* raw frame buffer address */
    unsigned char *lfb = set_display(&fb_info->width, &fb_info->height, &fb_info->pitch, &fb_info->isrgb);
    if (lfb != (unsigned char *)f->vnode->internal) {
        //free tmpfs_file_node *fnode
        kfree(f->vnode->internal);
        //store lfb instead
        f->vnode->internal = lfb;
    }
    tf->gprs[0] = 0;
    test_enable_interrupt();
}

syscall_t default_syscall_table[NUM_syscalls] = {
    [SYS_GETPID] = &syscall_getpid,
    [SYS_UART_RECV] = &syscall_uart_read,
    [SYS_UART_WRITE] = &syscall_uart_write,
    [SYS_EXEC] = &syscall_exec,
    [SYS_FORK] = &syscall_fork,
    [SYS_EXIT] = &syscall_exit,
    [SYS_MBOX] = &syscall_mbox_call,
    [SYS_KILL] = &syscall_kill,
    [SYS_SIGNAL] = &syscall_signal,
    [SYS_SIGKILL] = &syscall_sigkill,
    [SYS_SIGRETURN] = &syscall_sigreturn,
    [SYS_OPEN] = &syscall_open,
    [SYS_CLOSE] = &syscall_close,
    [SYS_WRITE] = &syscall_write,
    [SYS_READ] = &syscall_read,
    [SYS_MKDIR] = &syscall_mkdir,
    [SYS_MNT] = &syscall_mnt,
    [SYS_CHDIR] = &syscall_chdir,
    [SYS_LSEEK64] = &syscall_lseek64,
    [SYS_IOCTL] = &syscall_ioctl
};

void fork_test(){
    // printf("\nFork Test, pid %d\n", getpid());
    uart_write_string("\nFork Test, pid ");
    uart_write_no(getpid());
    uart_write_string("\n");;
    int cnt = 1;
    int ret = 0;
    if ((ret = fork()) == 0) { // child
        long long cur_sp;
        asm volatile("mov %0, sp" : "=r"(cur_sp));
        // printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
        uart_write_string("first child pid: ");
        uart_write_no(getpid());
        uart_write_string(", cnt: ");
        uart_write_no(cnt);
        uart_write_string(", ptr: 0x");
        uart_write_no_hex(&cnt);
        uart_write_string(", sp : 0x");
        uart_write_no_hex(cur_sp);
        uart_write_string("\n");
        ++cnt;

        if ((ret = fork()) != 0){
            asm volatile("mov %0, sp" : "=r"(cur_sp));
            // printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
            uart_write_string("first child pid: ");
            uart_write_no(getpid());
            uart_write_string(", cnt: ");
            uart_write_no(cnt);
            uart_write_string(", ptr: 0x");
            uart_write_no_hex(&cnt);
            uart_write_string(", sp : 0x");
            uart_write_no_hex(cur_sp);
            uart_write_string("\n");
        }
        else{
            while (cnt < 5) {
                asm volatile("mov %0, sp" : "=r"(cur_sp));
                // printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
                uart_write_string("second child pid: ");
                uart_write_no(getpid());
                uart_write_string(", cnt: ");
                uart_write_no(cnt);
                uart_write_string(", ptr: 0x");
                uart_write_no_hex(&cnt);
                uart_write_string(", sp : 0x");
                uart_write_no_hex(cur_sp);
                uart_write_string("\n");
                delay(1000000);
                ++cnt;
            }
        }
        exit();
    }
    else {
        // printf("parent here, pid %d, child %d\n", getpid(), ret);
        uart_write_string("parent here, pid ");
        uart_write_no(getpid());
        uart_write_string(", child ");
        uart_write_no(ret);
        uart_write_string("\n");
    }
}
