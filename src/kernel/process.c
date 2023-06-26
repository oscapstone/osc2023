#include "process.h"
#include "ds/list.h"
#include "mem/mem.h"
#include "thread.h"
#include "utils.h"
#include "peripherals/mini_uart.h"
#include "interrupt.h"
#include "type.h"
#include "mmu/mmu.h"
#include "fs/vfs.h"

struct Process_t init_process;
struct ds_list_head process_list;
extern long long _bss_end;
extern void load_all_eret();
extern struct filesystem tmpfs;
extern struct mount *rootfs;

extern void thread_exit();
extern void thread_check_zombie(void *);

extern struct file uart_file;
static uint32_t pid_counter = 0;

struct Thread_t * process_thread_create(void * func, void *arg, struct Process_t* proc, uint8_t kernel) {
    struct Thread_t *th = thread_create(func, arg);
    if(kernel == 1) {
    } else if(kernel == 0) {
        mappages(proc->ttbr0_el1, 0xffffffffb000ULL, (1 << 14), kernel_va2pa(th->sp - DEFAULT_STACK_SZ));
    } else if (kernel == 2) {
        // uart_send_string("Mapping Signal\r\n");
        mappages(proc->ttbr0_el1, 0xffffffff7000ULL, (1 << 14), kernel_va2pa(th->sp - DEFAULT_STACK_SZ));
    }
    // mappages(proc->ttbr0_el1, th->sp, )
    ds_list_addprev(&(proc->th_list), &(th->th_head));
    // th->sp = 0xfffffffff000ULL;
    // uart_send_string("In process thread create \r\n");
    // uart_send_u64(proc->ttbr0_el1);
    // uart_send_string("\r\n");
    th->proc = proc;
    th->saved_reg.ttbr0_el1 = proc->ttbr0_el1;
    return th;
}

void process_control_init() {
    ds_list_head_init(&(init_process.pl_head));
    ds_list_head_init(&(init_process.th_list));
    ds_list_head_init(&process_list);
    // create a init process, although in kernel space is weird;
    // the init process contain a idle thread that clean zombie thread and process
    init_process.ttbr0_el1 = 0x1000;
    struct Thread_t *th = process_thread_create(&thread_check_zombie, NULL, &init_process, 1);
    init_process.parent = NULL;
    init_process.pid = pid_counter ++;
    init_process.share_section = (void *)0x80000;
    init_process.share_section_sz = _bss_end;
    strncpy(init_process.name, "initd", 5);
    init_process.status = P_INIT;
    for(int i = 0; i < SIGNUMS; i ++) {
        init_process.signal[i] = 0;
    }

    for(int i = 0; i < P_FD_MAX; i ++) {
        init_process.files[i] = NULL;
    }
    init_process.cur_vnode = tmpfs.root;
    init_process.mnt = rootfs;
    ds_list_head_init(&(init_process.sig_list));
    ds_list_addnext(&(init_process.th_list), &(th->th_head));
    ds_list_addprev(&(process_list), &(init_process.pl_head));
    th->proc = &(init_process);
}
void _run_user_thread(void *prog, void *ret, void *stackbase) {
    void *base = (uint64_t)stackbase - 32 * 360;
    // disable_interrupt();

    // uart_switch_func(UART_DEFAULT);
    // uart_send_string("in _run user thread\r\n");
    // uart_switch_func(UART_ASYNC);
    struct Thread_t *cur = thread_get_current_instance();
    uint64_t ttbr0_el1 = cur->saved_reg.ttbr0_el1;
    asm volatile(
        "mov x0, #0\n"
        "mov x1, #0\n"
        "mov x2, #0\n"
        "mov x3, #0\n"
        "mov x4, #0\n"
        "mov x5, #0\n"
        "mov x6, #0\t\n"
        "mov x7, #0\t\n"
        "mov x8, #0\t\n"
        "mov x9, #0\t\n"
        "mov x10, #0\t\n"
        "mov x11, #0\t\n"
        "mov x12, #0\t\n"
        "mov x13, #0\t\n"
        "mov x14, #0\t\n"
        "mov x15, #0\t\n"
        "mov x16, #0\t\n"
        "mov x17, #0\t\n"
        "mov x18, #0\t\n"
        "mov x19, #0\t\n"
        "mov x20, #0\t\n"
        "mov x21, #0\t\n"
        "mov x22, #0\t\n"
        "mov x23, #0\t\n"
        "mov x24, #0\t\n"
        "mov x25, #0\t\n"
        "mov x26, #0\t\n"
        "mov x27, #0\t\n"
        "mov x28, #0\t\n"
    );
    asm volatile(
        "dsb ish\n"
        "msr ttbr0_el1, %[ttbr0]\n"
        "tlbi vmalle1is\n"
        "dsb ish\n"
        "isb\n"
        "mov sp, %[sp]\n"
        "mov lr, %[ret]\n"
        "msr elr_el1, %[elr]\n"
        "msr sp_el0, %[base_addr]\n"
        "mov x0, 0x0\n"
        "msr spsr_el1, x0\n"
        "eret"::[ret]"r"(ret), [elr]"r"(prog), [base_addr]"r"(base), [ttbr0]"r"(ttbr0_el1), [sp]"r"(stackbase)
    );
}
void run_user_thread() {
    struct Thread_t *th = thread_get_current_instance();

    _run_user_thread(th->entry, &thread_exit, 0xfffffffff000);
}




uint64_t process_exec(const char* name, char *const argv[], uint8_t kernel) {

    char *content;
    uint32_t filesize;

    struct file *f;
    int err = vfs_open(name, 0, &f);
    if(err) {
        return err;
    }
    struct fstat stat;
    err = vfs_stat(f, &stat);
    uint64_t flag = interrupt_disable_save();
    struct Thread_t *th = thread_get_current_instance();
    struct Process_t* cur_proc = th->proc;
    struct Process_t* new_proc = (struct Process_t*)kmalloc(sizeof(struct Process_t));

    filesize = stat.filesize;

    // setup fs info
    new_proc->cur_vnode = cur_proc->cur_vnode;
    new_proc->mnt = cur_proc->mnt;



    void *share_section = (void *)kmalloc((1 << 13) + filesize);
    new_proc->share_section_sz = (1 << 13) + filesize;
    new_proc->share_section = share_section;

    new_proc->parent = cur_proc->parent;
    new_proc->pid = pid_counter ++;


    for(int i = 0; i < P_FD_MAX; i ++) {
        if(i < 3) {
            new_proc->files[i] = &uart_file;
        } else {
            new_proc->files[i] = NULL;
        }
    }

    // copy code, data and bss
    int n = 0;
    while(n = vfs_read(f, new_proc->share_section + n, filesize)) {

        continue;
    }
    if(kernel == 0) {
        new_proc->ttbr0_el1 = kernel_va2pa((void *)cmalloc((1 << 12)));
        uint64_t addr = kernel_va2pa(new_proc->share_section);
        mappages(new_proc->ttbr0_el1, 0x0, new_proc->share_section_sz, addr);
    } else {
        new_proc->ttbr0_el1 = 0x1000;
    }

    ds_list_head_init(&(new_proc->sig_list));

    for(int i = 0; i < SIGNUMS; i ++) {
        new_proc->signal[i] = 0;
    }

    uint32_t namelen = strlen(name);
    memcpy(new_proc->name, name, namelen);
    new_proc->name[namelen] = '\0';
    // create a main thread for process
    // currently set argv NULL
    ds_list_head_init(&(new_proc->th_list));
    struct Thread_t *new_th = process_thread_create((thread_func)0, NULL, new_proc, kernel);
    new_th->proc= new_proc;

    new_th->entry = 0;

    
    new_th->saved_reg.lr = run_user_thread;

    new_proc->exit_code = 0;
    new_proc->handling_signal = 0;

    if(pid_counter > PID_MAX) {
        pid_counter = 1;
    }
    new_proc->status = P_INIT;

    // freeing all the thread belongs to original process
    struct ds_list_head *front = ds_list_front(&(cur_proc->th_list));
    struct Thread_t *th_en;
    while(front != &(cur_proc->th_list)) {
        th_en = container_of(front, struct Thread_t, th_head);
        th_en->status = TH_ZOMBIE;
        front = ds_list_front(front);
    }

    // add process to list
    ds_list_head_init(&(new_proc->pl_head));
    ds_list_addprev(&(process_list), &(new_proc->pl_head));

    interrupt_enable_restore(flag);
    while(1) {
        enable_interrupt();
        asm volatile("nop");
    }
}
void process_exit(int status) {
    uint64_t flag = interrupt_disable_save();
    struct Process_t *proc = process_get_current();
    struct ds_list_head *front = ds_list_front(&(proc->th_list));
    while(front != &(proc->th_list)) {
        struct Thread_t *th = container_of(front, struct Thread_t, th_head);
        th->status = TH_ZOMBIE;
        front = ds_list_front(front);
    }
    // enable_interrupt();
    // while(1) {
    //     asm volatile("nop");
    // }
    schedule(0);
    interrupt_enable_restore(flag);
}
void load_all_eret_log() {
    uart_send_string("In load all eret\r\n");
}

uint64_t process_fork(struct Trapframe_t *frame) {
    uint64_t flag = interrupt_disable_save();
    struct Thread_t *th = thread_get_current_instance();
    struct Process_t* cur_proc = th->proc;
    struct Process_t* new_proc = (struct Process_t*)kmalloc(sizeof(struct Process_t));
    new_proc->share_section = (char *)kmalloc(cur_proc->share_section_sz);
    new_proc->share_section_sz = cur_proc->share_section_sz;
    memcpy(new_proc->share_section, cur_proc->share_section, cur_proc->share_section_sz);



    // setup fs info
    new_proc->cur_vnode = cur_proc->cur_vnode;
    new_proc->mnt = cur_proc->mnt;

    for(int i = 0; i < P_FD_MAX; i ++) {
        new_proc->files[i] = cur_proc->files[i];
    }

    new_proc->ttbr0_el1 = kernel_va2pa((void *)cmalloc((1 << 12)));
    uint64_t addr = kernel_va2pa(new_proc->share_section);
    mappages(new_proc->ttbr0_el1, 0x0, cur_proc->share_section_sz, addr);

    new_proc->parent = cur_proc;

    // copy code, data and bss
    uint32_t namelen = strlen(cur_proc->name);
    memcpy(new_proc->name, cur_proc->name, namelen);
    new_proc->name[namelen] = '\0';

    ds_list_head_init(&(new_proc->th_list));

    struct Thread_t *new_th = process_thread_create((thread_func)0, NULL, new_proc, 0);
    new_proc->parent = cur_proc;
    new_proc->exit_code = 0;
    new_proc->pid = pid_counter ++;
    if(pid_counter > PID_MAX) {
        pid_counter = 1;
    }

    new_proc->status = P_INIT;


    ds_list_head_init(&(new_proc->pl_head));
    ds_list_addprev(&(process_list), &(new_proc->pl_head));



    // copy signal handler;
    ds_list_head_init(&(new_proc->sig_list));
    struct ds_list_head *front = ds_list_front(&(cur_proc->sig_list));
    if(front != NULL) {
        while(front != &(cur_proc->sig_list)) {
            struct Signal_t *sig = container_of(front, struct Signal_t, sig_head);
            struct Signal_t *new_sig = (struct Signal_t *)kmalloc(sizeof(struct Signal_t));
            new_sig->sig_num = sig->sig_num;
            new_sig->handler = sig->handler;
            new_sig->th = NULL;
            ds_list_head_init(&(new_sig->sig_head));
            ds_list_addprev(&(new_proc->sig_list), &(new_sig->sig_head));
            front = ds_list_front(front);
        }
    }

    for(int i = 0; i < SIGNUMS; i ++) {
        new_proc->signal[i] = 0;
    }


    // we need to set the reg
    // copy stack
    memcpy(new_th->sp - th->stack_sz, th->sp - th->stack_sz, th->stack_sz);
    uint64_t sp_offset = (uint64_t)(0xfffffffff000ULL) - (uint64_t)frame->sp;
    new_th->saved_reg.sp = (uint64_t)(0xfffffffff000ULL) - (uint64_t)sp_offset;

    uint64_t tmp = frame->elr_el1;
    uint64_t tmp_sp = frame->sp;
    frame->elr_el1 = frame->elr_el1;

    frame->sp = new_th->saved_reg.sp;
    new_th->saved_reg.sp = (0xfffffffff000ULL) - 32 * 10;
    uint64_t tmp_x0 = frame->x0;
    frame->x0 = 0;
    memcpy((void *)new_th->sp - 32 * 10, frame, 32 * 9);
    frame->elr_el1 = tmp;
    frame->sp = tmp_sp;
    frame->x0 = tmp_x0;
 

    new_proc->handling_signal = 0;
    new_th->saved_reg.lr = &load_all_eret;

    mmu_map_peripheral(new_proc->ttbr0_el1);

    interrupt_enable_restore(flag);

    return new_proc->pid;
}

struct Process_t *process_get_current() {
    struct Thread_t *th = thread_get_current_instance();

    return th->proc;
}

struct Process_t *create_process_instance() {
    uint64_t flag = interrupt_disable_save();
    struct Process_t *ret = (struct Process_t*)kmalloc(sizeof(struct Process_t));
    ret->exit_code = 0;
    ret->pid = pid_counter ++;
    ds_list_head_init(&(ret->pl_head));
    ds_list_head_init(&(ret->sig_list));
    ret->cur_vnode = tmpfs.root;
    ret->mnt = rootfs;
    for(int i = 0; i < SIGNUMS; i ++) {
        ret->signal[i] = 0;
    }
    for(int i = 0; i < P_FD_MAX; i ++) {
        ret->files[i] = NULL;
    }
    ds_list_addprev(&(process_list), &(ret->pl_head));
    ds_list_head_init(&(ret->th_list));
    interrupt_enable_restore(flag);
    return ret;
}

void process_free(struct Process_t *proc) {
    struct ds_list_head *front = ds_list_front(&(proc->th_list));
    if(front == NULL) {
        uint64_t flag = interrupt_disable_save();

        ds_list_remove(&(proc->pl_head));
        proc->exit_code = 0;

        kfree(proc);
        interrupt_enable_restore(flag);
    }
}

struct Process_t *get_process_from_pid(uint32_t pid) {
    struct ds_list_head *front = ds_list_front(&process_list);
    if(front == NULL) {
        return NULL;
    }

    uint64_t val = 0;
    while(front != (&(process_list))) {
        struct Process_t *proc = container_of(front, struct Process_t, pl_head);

        if(proc->pid == pid) {
            return proc;
        }
        front = ds_list_front(front);
        val += 1;

    }

    return NULL;
}