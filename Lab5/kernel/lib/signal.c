#include "signal.h"
#include "sched.h"
#include "memory.h"

void check_signal(trapframe_t *tpf) {
    enter_critical();

    if (cur_thread->signal_is_checking) {
        exit_critical();
        return;
    }
    // prevent nested running signal handler
    cur_thread->signal_is_checking = 1;    
    exit_critical();
    
    for (int i = 0; i <= SIGNAL_MAX; i++) {
        store_context(&cur_thread->signal_saved_context); // why
        if (cur_thread->sigcount[i] > 0) {

            enter_critical();
            
            cur_thread->sigcount[i]--;
            
            exit_critical();
            
            run_signal(tpf, i);
        }
    }

    enter_critical();
    
    cur_thread->signal_is_checking = 0;
    
    exit_critical();
}

void run_signal(trapframe_t *tpf, int signal) {
    cur_thread->cur_signal_handler = cur_thread->signal_handler[signal];

    // run default handler in kernel
    if (cur_thread->cur_signal_handler == signal_default_handler) {
        signal_default_handler();
        return;
    }

    char * temp_signal_userstack = malloc(USTACK_SIZE);

    asm("msr elr_el1, %0\n\t"
        "msr sp_el0, %1\n\t"
        "msr spsr_el1, %2\n\t"
        "eret\n\t" ::"r"(signal_handler_wrapper),
        "r"(temp_signal_userstack + USTACK_SIZE),
        "r"(tpf->spsr_el1));
}

void signal_handler_wrapper() {
    cur_thread->cur_signal_handler();
    // system call sigreturn
    asm("mov x8,50\n\t"
        "svc 0\n\t");
}

void signal_default_handler() {
    kill(0, cur_thread->id);
}