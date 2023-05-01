#include "signal.h"
#include "thread.h"
#include "exception.h"
#include "syscall.h"
#include "uart.h"
static void default_sig_handler()
{
    uart_write_string("This is default signal handler\n");
}
static void sig_kill_handler()
{
    _exit(9);
}

signal_handler_t default_sig_handlers[MAX_SIGNAL+1] = {
    [0 ... SIG_KILL-1] = (signal_handler_t)&default_sig_handler,
    [SIG_KILL] = &sig_kill_handler,
    [SIG_KILL+1 ... MAX_SIGNAL] = (signal_handler_t)&default_sig_handler
};

void handle_current_signal()
{
    disable_interrupt();
    task_t *current = get_current_thread();
    struct trap_frame *current_tf = current->tf;
    while (!list_empty(&(current->pending_signal_list))) {
        struct signal *sig = list_entry(current->pending_signal_list.next, struct signal, node);
        if (sig->handling) break;
        sig->handling = 1;
        memcpy(sig->tf, current_tf, sizeof(struct trap_frame));
        test_enable_interrupt();
        //multiplex signal
        
        if (current->reg_sig_handlers[sig->signum] == NULL) {
            //user doesn't register user-defined handler, use default signal handler
            default_sig_handlers[sig->signum]();
            syscall_sigreturn(current_tf);
        } else {
            //run user-defined signal handler
            _run_user_prog(current->reg_sig_handlers[sig->signum], sigret, STACK_BASE(sig->handler_user_stack, sig->handler_user_stack_size));
        }
        disable_interrupt();
    }
    test_enable_interrupt();
}