#include "syscall.h"
#include "sched.h"
#include "signal.h"
#include "uart.h"
#include "mbox.h"
#include "memory.h"
#include "interrupt.h"
#include "cpio.h"
#include "ctx.h"
#include "string.h"

int get_pid(trapframe_t * tpf) {
    tpf->x0 = cur_thread->id;
    return cur_thread->id;
}

size_t uart_read(trapframe_t * tpf, char buf[], size_t size) {
    int i;
    for (i = 0; i < size; i++) {
        buf[i] = uart_async_getc();
    }
    tpf->x0 = i;
    return i;
}

size_t uart_write(trapframe_t * tpf, const char buf[], size_t size) {
    int i;
    for (i = 0; i < size; i++) {
        uart_async_putc(buf[i]);
    }
    tpf->x0 = i;
    return i;
}

int exec(trapframe_t * tpf, const char * name, char * const argv[]) {
    char * filestart = get_file_start((char *)name);
    unsigned int filesize = get_file_size((char *)name);
    cur_thread->datasize = filesize;
    memcpy(cur_thread->data, filestart, filesize);

    for (int i = 0; i <= SIGNAL_MAX; i++) {
        cur_thread->signal_handler[i] = signal_default_handler;
    }
    
    tpf->elr_el1 = (unsigned long)cur_thread->data; // position to go to in EL0
    tpf->sp_el0 = (unsigned long)cur_thread->user_sp + USTACK_SIZE; // stack pointer in EL0
    tpf->x0 = 0;
    return 0;
}

int fork(trapframe_t *tpf) {
    enter_critical();
    thread_t *new_thread = thread_create(cur_thread->data);

    // store parent
    int parent_pid = cur_thread->id;
    thread_t * parent_thread = cur_thread;

    // copy signal handler
    for (int i = 0; i <= SIGNAL_MAX; i++) {
        new_thread->signal_handler[i] = cur_thread->signal_handler[i];
    }

    // copy data into new process
    new_thread->datasize = cur_thread->datasize;
    new_thread->data = malloc(cur_thread->datasize * sizeof(char));
    memcpy(new_thread->data, cur_thread->data, cur_thread->datasize);

    // copy user stack into new process
    memcpy(new_thread->user_sp, cur_thread->user_sp, USTACK_SIZE);

    // copy kernel stack into new process
    memcpy(new_thread->kernel_sp, cur_thread->kernel_sp, KSTACK_SIZE);

    store_context(current_ctx); // set child lr to here
    // for child
    if (parent_pid == cur_thread->id) {
        // parent
        new_thread->context.x19 = cur_thread->context.x19;
        new_thread->context.x20 = cur_thread->context.x20;
        new_thread->context.x21 = cur_thread->context.x21;
        new_thread->context.x22 = cur_thread->context.x22;
        new_thread->context.x23 = cur_thread->context.x23;
        new_thread->context.x24 = cur_thread->context.x24;
        new_thread->context.x25 = cur_thread->context.x25;
        new_thread->context.x26 = cur_thread->context.x26;
        new_thread->context.x27 = cur_thread->context.x28;
        new_thread->context.x28 = cur_thread->context.x28;
        new_thread->context.fp = cur_thread->context.fp + new_thread->kernel_sp - cur_thread->kernel_sp; // move fp
        new_thread->context.lr = cur_thread->context.lr;
        new_thread->context.sp = cur_thread->context.sp + new_thread->kernel_sp - cur_thread->kernel_sp; // move kernel sp
        
        exit_critical();

        tpf->x0 = new_thread->id;
        return new_thread->id;
    }
    else {
        // child
        tpf = (trapframe_t *)((char *)tpf + (unsigned long)new_thread->kernel_sp - (unsigned long)parent_thread->kernel_sp); // move tpf
        tpf->sp_el0 += new_thread->user_sp - parent_thread->user_sp;
        tpf->x0 = 0;
        return 0;
    }
}


void exit(trapframe_t * tpf, int status) {
    thread_exit();
}

int syscall_mbox_call(trapframe_t *tpf, unsigned char ch, unsigned int *mbox) {
    enter_critical();

    /* Combine the message address (upper 28 bits) with channel number (lower 4 bits) */
    unsigned long r = (((unsigned long)((unsigned long)mbox) & ~0xF) | (ch & 0xF));

    /* wait until we can write to the mailbox */
    do {
        asm volatile("nop");
    } while (*MBOX_STATUS & MBOX_FULL);

    /* write the address of our message to the mailbox with channel identifier */
    *MBOX_WRITE = r;

    /* now wait for the response */
    while (1) {
        /* is there a response? */
        do {
            asm volatile("nop");
        } while (*MBOX_STATUS & MBOX_EMPTY);

        /* is it a response to our message? */
        if (r == *MBOX_READ) {
            /* is it a valid successful response? */
            tpf->x0 = (mbox[1] == MBOX_RESPONSE);
            exit_critical();
            return mbox[1] == MBOX_RESPONSE;
        }
    }

    tpf->x0 = 0;
    
    exit_critical();

    return 0;
}

void kill(trapframe_t * tpf, int pid) {
    enter_critical();

    if (pid < 0 || pid >= PIDMAX || threads[pid].status == FREE) {
        exit_critical();
        return;
    }
    threads[pid].status = DEAD;

    exit_critical();
    
    schedule();
}

void signal_register(int signal, void (*handler)()) {
    if (signal > SIGNAL_MAX || signal < 0) {
        return;
    }
    cur_thread->signal_handler[signal] = handler;
}

void signal_kill(int pid, int signal) {
    if (pid >= PIDMAX || pid < 0 || threads[pid].status == FREE) {
        return;
    }
    enter_critical();
    threads[pid].sigcount[signal]++;
    exit_critical();
}

void signal_return(trapframe_t *tpf) { // why
    unsigned long signal_ustack = (tpf->sp_el0 % USTACK_SIZE == 0) ? tpf->sp_el0 - USTACK_SIZE : tpf->sp_el0 & (~(USTACK_SIZE - 1));
    free((char *)signal_ustack);
    load_context(&cur_thread->signal_saved_context);
}

