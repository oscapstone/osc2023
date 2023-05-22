#include "uart.h"
#include "exception.h"
#include "timer.h"
#include "irqtask.h"
#include "syscall.h"
#include "sched.h"
#include "signal.h"
#include "mmu.h"
#include "stddef.h"

// For svc 0 (supervisor call)
void sync_64_router(trapframe_t *tpf, unsigned long x1)
{
    //uart_printf("syscall no : %d\r\n", tpf->x8);
    //uart_printf("elr_el1    : 0x%x\r\n", tpf->elr_el1);
    //uart_printf("sp_el0    : 0x%x\r\n", tpf->sp_el0);
    //uart_printf("spsr_el1    : 0x%x\r\n", tpf->spsr_el1);
    //while(1);

    //For MMU page fault
    esr_el1_t *esr;
    esr = (esr_el1_t *)&x1;
    if (esr->ec == DATA_ABORT_LOWER || esr->ec == INS_ABORT_LOWER)
    {
        handle_abort(esr);
        return;
    }

    enable_interrupt();
    unsigned long long syscall_no = tpf->x8;

    if (syscall_no == 0)
    {
        getpid(tpf);
    }
    else if(syscall_no == 1)
    {
        uartread(tpf,(char *) tpf->x0, tpf->x1);
    }
    else if (syscall_no == 2)
    {
        uartwrite(tpf,(char *) tpf->x0, tpf->x1);
    }
    else if (syscall_no == 3)
    {
        exec(tpf,(char *) tpf->x0, (char **)tpf->x1);
    }
    else if (syscall_no == 4)
    {
        fork(tpf);
    }
    else if (syscall_no == 5)
    {
        exit(tpf,tpf->x0);
    }
    else if (syscall_no == 6)
    {
        syscall_mbox_call(tpf,(unsigned char)tpf->x0, (unsigned int *)tpf->x1);
    }
    else if (syscall_no == 7)
    {
        kill(tpf, (int)tpf->x0);
    }
    else if (syscall_no == 8)
    {
        signal_register(tpf->x0, (void (*)())tpf->x1);
    }
    else if (syscall_no == 9)
    {
        signal_kill(tpf->x0, tpf->x1);
    }else if(syscall_no == 10)
    {
        sys_mmap(tpf,(void *)tpf->x0,tpf->x1,tpf->x2,tpf->x3,tpf->x4,tpf->x5);
    }
    else if (syscall_no == 11)
    {
        sys_open(tpf, (char*)tpf->x0, tpf->x1);
    }
    else if (syscall_no == 12)
    {
        sys_close(tpf, tpf->x0);
    }
    else if (syscall_no == 13)
    {
        sys_write(tpf, tpf->x0, (char *)tpf->x1, tpf->x2);
    }
    else if (syscall_no == 14)
    {
        sys_read(tpf, tpf->x0, (char *)tpf->x1, tpf->x2);
    }
    else if (syscall_no == 15)
    {
        sys_mkdir(tpf, (char *)tpf->x0, tpf->x1);
    }
    else if (syscall_no == 16)
    {
        sys_mount(tpf, (char *)tpf->x0, (char *)tpf->x1, (char *)tpf->x2, tpf->x3, (void*)tpf->x4);
    }
    else if (syscall_no == 17)
    {
        sys_chdir(tpf, (char *)tpf->x0);
    }
    else if(syscall_no == 18)
    {
        sys_lseek64(tpf, tpf->x0, tpf->x1, tpf->x2);
    }
    else if(syscall_no == 19)
    {
        // ioctl 0 will be use to get info
        // there will be default value in info
        // if it works with default value, you can ignore this syscall
        sys_ioctl(tpf, tpf->x0, tpf->x1, (void*)tpf->x2);
        tpf->x0 = 0;
    }
    else if (syscall_no == 50)
    {
        sigreturn(tpf);
    }else
    {
        uart_printf("unknown syscall\r\n");
        while(1);
    }

    // load all shouldn't be irq
    disable_interrupt();
    /*
    unsigned long long spsr_el1;
	__asm__ __volatile__("mrs %0, SPSR_EL1\n\t" : "=r" (spsr_el1));

    unsigned long long elr_el1;
	__asm__ __volatile__("mrs %0, ELR_EL1\n\t" : "=r" (elr_el1));

    unsigned long long esr_el1;
	__asm__ __volatile__("mrs %0, ESR_EL1\n\t" : "=r" (esr_el1));*/

    //uart_printf("exception sync_el0_64_router -> spsr_el1 : 0x%x, elr_el1 : 0x%x, esr_el1 : 0x%x\r\n",spsr_el1,elr_el1,esr_el1);
}

void irq_router(trapframe_t* tpf)
{
    //uart_printf("ena : %d\r\n", is_disable_interrupt());
    //uart_printf("irq_basic_pending: %x\n",*IRQ_BASIC_PENDING);
    //uart_printf("irq_pending_1: %x\n",*IRQ_PENDING_1);
    //uart_printf("irq_pending_2: %x\n",*IRQ_PENDING_2);
    //uart_printf("source : %x\n",*CORE0_INTERRUPT_SOURCE);

    //目前實測能從pending_1 AUX_INT, CORE0_INTERRUPT_SOURCE=GPU 辨別其他都是0(或再找)

    if (*IRQ_PENDING_1 & IRQ_PENDING_1_AUX_INT && *CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_GPU) // from aux && from GPU0 -> uart exception
    {
        //https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf p13
        /*
        AUX_MU_IIR
        on read bits[2:1] :
        00 : No interrupts
        01 : Transmit holding register empty
        10 : Receiver holds valid byte
        11: <Not possible> 
        */

        // buffer read, write
        if (*AUX_MU_IIR & (0b01 << 1)) //can write
        {
            disable_mini_uart_w_interrupt(); // lab 3 : advanced 2 -> mask device line (enable by handler)
            add_task(uart_interrupt_w_handler, UART_IRQ_PRIORITY);
            run_preemptive_tasks();
        }
        else if (*AUX_MU_IIR & (0b10 << 1)) // can read
        {   
            //不知道為啥關了還會進來 這種時候清除FIFO就可以關掉 (會有最多8個FIFO裡的byte消失)
            if (!mini_uart_r_interrupt_is_enable())
            {
                *AUX_MU_IIR = 0xC2;
                return;
            }
            
            //uart_printf("dd %d %d \r\n", mini_uart_r_interrupt_is_enable(), mini_uart_w_interrupt_is_enable());
            disable_mini_uart_r_interrupt(); // lab 3 : advanced 2 -> mask device line (enable by handler)
            add_task(uart_interrupt_r_handler, UART_IRQ_PRIORITY);
            run_preemptive_tasks();
        }
        else
        {
            uart_printf("uart handler error\r\n");
        }

    }else if(*CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_CNTPNSIRQ)  //from CNTPNS (core_timer)
    {
        core_timer_disable(); // lab 3 : advanced 2 -> mask device line
        add_task(core_timer_handler, TIMER_IRQ_PRIORITY);
        run_preemptive_tasks();
        core_timer_enable(); // lab 3 : advanced 2 -> unmask device line

        disable_interrupt();
        //at least two thread running -> schedule for any timer irq
        if (run_queue->next->next != run_queue)schedule();
    }

    //only do signal handler when return to user mode
    if ((tpf->spsr_el1 & 0b1100) == 0)
    {
        check_signal(tpf);
    }

    // load all shouldn't be irq
    disable_interrupt();
}

void invalid_exception_router(unsigned long long x0){
    unsigned long long elr_el1;
    __asm__ __volatile__("mrs %0, ELR_EL1\n\t"
                         : "=r"(elr_el1));

    unsigned long long spsr_el1;
    __asm__ __volatile__("mrs %0, SPSR_EL1\n\t"
                         : "=r"(spsr_el1));

    unsigned long long lr;
    __asm__ __volatile__("mov %0, lr\n\t"
                         : "=r"(lr));

    uart_printf("invalid exception : 0x%x\r\n", elr_el1);
    uart_printf("invalid exception : 0x%x\r\n", spsr_el1);
    uart_printf("invalid exception : 0x%x\r\n", lr);
    uart_printf("invalid exception : x0 : %x\r\n",x0);
    while(1);
}

//https://developer.arm.com/documentation/ddi0595/2020-12/AArch64-Registers/DAIF--Interrupt-Mask-Bits
//https://www.twblogs.net/a/5b7c4ca52b71770a43da534e
//zero -> enable
//others -> disable
unsigned long long is_disable_interrupt()
{
    unsigned long long daif;
    __asm__ __volatile__("mrs %0, daif\n\t"
                         : "=r"(daif));

    return daif;  //enable -> daif == 0 (no mask)
}

static unsigned long long lock_count = 0;
void lock()
{
    disable_interrupt();
    lock_count++;
}

void unlock()
{
    lock_count--;
    if (lock_count<0)
    {
        uart_printf("lock error !!!\r\n");
        while(1);
    }
    if (lock_count == 0)
        enable_interrupt();

}