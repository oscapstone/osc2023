#include "signal.h"
#include "syscall.h"
#include "sched.h"
#include "mm.h"

extern thread_t *curr_thread;



// Check if there are signals waiting to be processed
void check_signal(trapframe_t *tpf)
{
    // Check if the current thread is processing other signals. If yes, return directly without processing
    if(curr_thread->signal_inProcess) return;
    lock();

    // Set the current thread to handle  signals to prevent errors during recursive calls
    curr_thread->signal_inProcess = 1;
    unlock();

    // Search for all signals
    for (int i = 0; i <= SIGNAL_MAX; i++)
    {
        // Store the state of the current thread
        store_context(&curr_thread->signal_savedContext);

        // If the number of current signals is greater than 0, process the signal
        if(curr_thread->sigcount[i]>0)
        {
            lock();
            curr_thread->sigcount[i]--;
            unlock();
            run_signal(tpf, i);
        }
    }
    lock();
    curr_thread->signal_inProcess = 0;
    unlock();  
}

// Signal handler function for performing registration
void run_signal(trapframe_t *tpf, int signal)
{
    // Set the currently executing signal handler
    curr_thread->curr_signal_handler = curr_thread->signal_handler[signal];

    // If it is a signal default handler, it will be executed in kernel mode
    if (curr_thread->curr_signal_handler == signal_default_handler)
    {
        signal_default_handler();
        return;
    }

    // Run registered handler in userspace
    char *temp_signal_userstack = kmalloc(USTACK_SIZE); // allocate a userspace stack
    asm("msr elr_el1, %0\n\t"
        "msr sp_el0, %1\n\t"
        "msr spsr_el1, %2\n\t"
        "eret\n\t" ::"r"(signal_handler_wrapper),
        "r"(temp_signal_userstack + USTACK_SIZE),
        "r"(tpf->spsr_el1)); // Switch to the specified signal handler and use the allocated userspace stack

}

// Used to make the system call sigreturn after the signal processing function returns. 
// This system call is used to switch the system context back to the reply of the upper number processing when the signal is published.
void signal_handler_wrapper()
{
    (curr_thread->curr_signal_handler)();
    // Trigger a system call sigreturn
    asm("mov x8,50\n\t"
        "svc 0\n\t");

    // https://developer.arm.com/documentation/dui0473/m/arm-and-thumb-instructions/svc
}

// A default signal handler to use if no registered signal handler exists.
void signal_default_handler()
{
    kill(0,curr_thread->pid);
}

