#include <preempt.h>
#include <sched.h>
#include <current.h>

void preempt_disable(){
    uint32 daif;
    
    daif = save_and_disable_interrupt();

    current->preempt += 1;

    restore_interrupt(daif);
}

void preempt_enable(){
    uint32 daif;

    daif = save_and_disable_interrupt();

    current->preempt -= 1;

    restore_interrupt(daif);
}