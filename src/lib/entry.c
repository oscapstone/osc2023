#include <arm.h>
#include <entry.h>
#include <syscall.h>
#include <mmu.h>
#include <panic.h>

void el0_sync_handler(trapframe *regs, uint32 syn){
    esr_el1_t *esr;

    esr = (esr_el1_t *)&syn;

    switch(esr->ec){
        case EC_SVC_64:
            syscall_handler(regs);
            break;
        // instruction abort
        case EC_IA_LE:
        // data abort
        case EC_DA_LE:
            mem_abort(esr);
            break;
        default:
            show_trapframe(regs);
            panic("esr->ec: %x", esr->ec);
    }
}