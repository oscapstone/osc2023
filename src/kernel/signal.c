#include "signal.h"
#include "thread.h"
#include "ds/list.h"
#include "utils.h"
#include "peripherals/mini_uart.h"
#include "interrupt.h"
#include "process.h"
#include "waitqueue.h"
#include "process.h"

extern void syscall_sigreturn();




struct Signal_t *signum2handler(uint8_t signal, struct Process_t *proc) {
    struct ds_list_head *front = ds_list_front(&(proc->sig_list));
    if(front == NULL) {
        return NULL;
    }

    while(front != (&(proc->sig_list))) {
        struct Signal_t *sig = container_of(front, struct Signal_t, sig_head);
        if(sig->sig_num == signal) {
            return sig;
        }
        front = ds_list_front(front);
    }
    return NULL;
}

void signal_register(uint8_t signal, signal_handler handler) {

    struct Process_t *proc = process_get_current();
    struct Signal_t *sig = signum2handler(signal, proc);
    if(sig != NULL) {
        sig->handler = handler;
    } else {

        sig = (struct Signal_t *)kmalloc(sizeof(struct Signal_t));
        ds_list_head_init(&(sig->sig_head));
        sig->sig_num = signal;
        sig->handler = handler;
        ds_list_addprev(&(proc->sig_list), &(sig->sig_head));
    }
}

void signal_kill(unsigned int pid, uint8_t signal) {
    struct Process_t *proc = get_process_from_pid(pid);
;
    if(proc == NULL) {
        return;
    }
    if(signal > SIGNUMS || signal < 1) {
        return;
    }

    proc->signal[signal] += 1;
}

void signal_handler_default() {
    uart_send_string("Default handler\r\n");
    return;
}

void signal_kill_default() {
    process_exit(0);
    return;
}

void run_signal_thread() {

    struct Thread_t *th = thread_get_current_instance();

    _run_user_thread(th->entry, 0xffffffff6000ULL, 0xffffffffb000ULL);
    // _run_user_thread(th->entry, &syscall_sigreturn, 0xffffffffb000ULL);
}
void signal_call_handler(struct Process_t *proc, uint8_t signum) {
    struct Signal_t *sig = signum2handler(signum, proc);
    signal_handler handler;
    if(sig == NULL) {
        if(signum == 9) {
            handler = signal_kill_default;
        } else {
            handler = signal_handler_default;
        }
        // run default handler in el1
        uint64_t flag = interrupt_disable_save();
        handler();
        interrupt_enable_restore(flag);
        // syscall_sigreturn();
    } else {
        handler = sig->handler;
        struct Thread_t *th = process_thread_create(handler, NULL, proc, 2);
        sig->sig_return = (void*)kmalloc((1 << 12));
        memcpy(sig->sig_return, &syscall_sigreturn, (1 << 8));
        mappages(th->saved_reg.ttbr0_el1, 0xffffffff6000, (1 << 12), kernel_va2pa(sig->sig_return));
        th->saved_reg.lr = run_signal_thread;
        struct Thread_t *wait_th = thread_get_current_instance();
        sig->th = wait_th;
        waitthread(thread_get_current_instance());
    }
    return;
}

void signal_check() {
    struct Process_t *proc = process_get_current();
    if(proc->handling_signal != 0) return;
    for(int i = 0; i < SIGNUMS; i ++) {
        if(proc->signal[i] > 0) {
            // first set to 0 otherwise other thread for the same process may run the same handler

            proc->handling_signal = i;
            signal_call_handler(proc, i);
        }
    }
}

void signal_sigreturn() {
    uint64_t flag = interrupt_disable_save();
    struct Thread_t *th = thread_get_current_instance();
    struct Process_t *proc = th->proc;
    struct Signal_t *sig = signum2handler(proc->handling_signal, proc);
    wakeupthread(sig->th);
    sig->th = NULL;
    th->status = TH_ZOMBIE;
    proc->handling_signal = 0;
    proc->signal[sig->sig_num] -= 1;
    schedule(0);
    interrupt_disable_save(flag);
}


