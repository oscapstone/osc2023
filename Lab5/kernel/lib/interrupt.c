#include "uart.h"
#include "timer.h"
#include "task.h"
#include "interrupt.h"
#include "sched.h"

extern list_head_t * ready_queue;

void enable_interrupt() {
    asm volatile("msr DAIFClr, 0xf");
}

void disable_interrupt() {
    asm volatile("msr DAIFSet, 0xf");
}

void irq_handler(unsigned long long x0) {
    if (*CORE0_INT_SRC & CORE0_INT_SRC_GPU && *IRQ_PENDING_1 & IRQ_PENDING_1_AUX_INT) { // uart interrupt: gpu, aux
        if (*AUX_MU_IIR & (0b01 << 1)) {
            // Transmit holding register empty
            disable_uart_tx_interrupt();
            add_task(uart_tx_interrupt_handler, UART_IRQ_PRIORITY);
            pop_task();
        }
        else if (*AUX_MU_IIR & (0b10 << 1)) {
            // Recevier holding valid byte
            disable_uart_rx_interrupt();
            add_task(uart_rx_interrupt_handler, UART_IRQ_PRIORITY);
            pop_task();
        }
        else {
            uart_printf("uart interrupt error\n");
        }
    }
    else if (*CORE0_INT_SRC & CORE0_INT_SRC_TIMER) { // timer interrupt
        core_timer_interrupt_disable();
        add_task(core_timer_handler, TIMER_IRQ_PRIORITY);
        pop_task();
        core_timer_interrupt_enable();

        if (ready_queue->next->next != ready_queue) {
            schedule();
        }
    }
}

void invalid_exception_handler(unsigned long long x0) {
    uart_printf("invalid handler 0x%x\n", x0);
    uart_getc(); // halt
}

void sync_el0_64_handler(trapframe_t * tpf) {
    enable_interrupt();
    unsigned long long syscall_no = tpf->x8;
    // uart_printf("dbg: sync_el0_64_handler start, syscall_no = %d\n", syscall_no);

    switch (syscall_no) {
    case 0:
        get_pid(tpf);
        break;
    case 1:
        uart_read(tpf, (char *)tpf->x0, tpf->x1);
        break;
    case 2:
        uart_write(tpf, (char *)tpf->x0, tpf->x1);
        break;
    case 3:
        exec(tpf, (char *)tpf->x0, (char **)tpf->x1);
        break;
    case 4:
        fork(tpf);
        break;
    case 5:
        exit(tpf, tpf->x0);
        break;
    case 6:
        syscall_mbox_call(tpf, (unsigned char)tpf->x0, (unsigned int *)tpf->x1);
        break;
    case 7:
        kill(tpf, (int)tpf->x0);
        break;
    case 8:
        signal_register(tpf->x0, (signal_handler_t)tpf->x1);
        break;
    case 9:
        signal_kill(tpf->x0, tpf->x1);
        break;
    case 50:
        signal_return(tpf);
        break;
    }
    // uart_printf("dbg: sync_el0_64_handler start\n");
}

void set_cpacr_el1() {
    asm volatile(
        "mov x1, (3 << 20)\n\t"
        "msr CPACR_EL1, x1\n\t"
    );
}

static unsigned long long lock_count = 0;

void enter_critical() {
    disable_interrupt();
    lock_count++;
}

void exit_critical() {
    lock_count--;
    if (lock_count < 0) {
        uart_printf("lock error !!!\r\n");
        while(1);
    }
    if (lock_count == 0) {
        enable_interrupt();
    }
}