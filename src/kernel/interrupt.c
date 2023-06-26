#include "interrupt.h"
#include "peripherals/mini_uart.h"
#include "peripherals/base.h"
#include "peripherals/mini_uart.h"
#include "utils.h"
#include "test/demo.h"
#include "time.h"
#include "type.h"
#include "event.h"
#include "thread.h"
#include "process.h"
#include "syscall.h"

#define IRQ_BASIC_PENDING (PBASE + 0xB200)
#define IRQ_GPU_PENDING1 (PBASE + 0xB204)
#define IRQ_GPU_PENDING2 (PBASE + 0xB208)
#define CORE_IRQ_SOURCE (0xffff000040000060)
#define GPU_INTERRUPT (0x100)
#define AUX_INT (1 << 29)

int interrupt_counter = 0;
int interrupt_handler_counter = 0;
extern struct k_event demo_timer_event;
int demo_state = 0;

void irqhandler_inc() {
    int flag = interrupt_disable_save();
    interrupt_handler_counter += 1;
    interrupt_enable_restore(flag);
}

void irqhandler_dec() {
    int flag = interrupt_disable_save();
    interrupt_handler_counter -= 1;
    interrupt_enable_restore(flag);
}
int irqhandler_cnt_get() {
    return interrupt_handler_counter;
}

void enable_interrupt() {
    asm volatile ("msr DAIFClr, 0xf\n");
}
void disable_interrupt() {
    asm volatile ("msr DAIFSet, 0xf\n");
}

void exception_entry(unsigned long esr, unsigned long elr, unsigned long spsr) {
    uart_switch_func(UART_DEFAULT);
    uart_send_string("ESR: ");
    uart_send_u64(esr);
    uart_send_string("\r\n");
    uart_send_string("ELR: ");
    uart_send_u64(elr);
    uart_send_string("\r\n");
    uart_send_string("SPSR: ");
    uart_send_u64(spsr);
    uart_send_string("\r\n");
    uart_switch_func(UART_ASYNC);
    while(1) {
        asm volatile("nop\n");
    }
    return;
}

void time_interrupt_handler(struct Trapframe_t *frame) {
    struct k_timeout *t;
    uint64_t tick;
    while(1) {
        asm volatile("mrs %0, cntpct_el0\n":"=r"(tick));
        t = k_timeout_queue_front();
        if(t == NULL) {
            asm volatile(
                "ldr x0, =0xffffffffff\n"
                "msr cntp_cval_el0, x0\n"
            );
            break;
        }
        if(t->endtick <= tick) {
            t->cb(t->arg);
            k_timeout_queue_pop();
        } else {
            // setup new timeout if endtick is less
            uint64_t x = t->endtick;
            asm volatile(
                "msr cntp_cval_el0, %0\n":"=r"(x)
            );
            break;
        }
    }
    handle_time_schedule(frame);
}

void uart_handler() {
    uint32_t pending = (*(volatile unsigned int*)IRQ_GPU_PENDING1);
    if(pending & AUX_INT) {
        unsigned int IER_REG = *(volatile unsigned int*)(AUX_MU_IER_REG);
        if(IER_REG & 1) {
            // only recv can delay the work
            irqhandler_inc();
            uart_int_recv_handler();
        }
        if(IER_REG & 2) {
            uart_int_trans_handler();
        }
    }
}
void spx_irq_handler(struct Trapframe_t *frame, uint64_t esr) {
    unsigned int pending = read_reg_32(CORE_IRQ_SOURCE);
    if(pending & GPU_INTERRUPT) {

        uart_handler();
    }
    else if(pending & 0x2) {
        time_interrupt_handler(frame);
    }
    else {
            // uint64_t dfsr_value;
            // asm volatile ("mrs %0, dfsr_el1" : "=r" (dfsr_value));

        }

    return;
}

void lower_irq_handler(struct Trapframe_t *frame) {
    unsigned int pending = read_reg_32(CORE_IRQ_SOURCE);
    if(pending & GPU_INTERRUPT) {
        uart_handler();
    }
    else if(pending & 0x2) {

        time_interrupt_handler(frame);
    }
    return;
}
void interrupt_enable_restore(uint64_t flag) {
    uint64_t x = flag;
    asm volatile(
        "msr DAIF, %0\n":"=r"(x)
    );
}

uint64_t interrupt_disable_save() {
    uint64_t flag;
    asm volatile(
        "mrs %0, DAIF\n":"=r"(flag)
    );
    asm volatile(
        "msr DAIFSet, 0xf\n"
    );
    return flag;
}


struct Trapframe_t anoFrame;
void svc_handler(struct Trapframe_t *frame) {
    if(frame->esr_el1 >> 26 == 0b010101) {
        // memcpy(&anoFrame, frame, sizeof(struct Trapframe_t));
        uint16_t svcnum = frame->esr_el1 & 0xffff;
        if(svcnum == 0) {
            syscall_handler(frame);
        }
        // for(int i = 0; i < sizeof(struct Trapframe_t) >> 3; i ++) {
        //     if(*((uint64_t*)(&anoFrame) + i) != *((uint64_t*)(frame) + i)) {
        //         uart_switch_func(UART_DEFAULT);
        //         // uart_send_string("fucked\r\n");
        //         uart_send_string("In i = ");
        //         uart_send_dec(i);
        //         uart_send_string("\r\n");
        //         uart_switch_func(UART_ASYNC);
        //     }
        // }
    } else {
        // uart_switch_func(UART_DEFAULT);
        // uart_send_string("\r\nOther interrupt\r\n");
        // // uart_send_u64(frame->esr_el1);
        // // uart_send_string("\r\n");
        // // uart_send_u64(frame->elr_el1);
        // uart_send_u64(thread_get_current_instance()->tid);
        // uart_send_string("\r\n");
        // // }
        // uart_switch_func(UART_ASYNC);
    }
}

uint64_t check_exception_depth() {
    struct Thread_t *th = thread_get_current_instance();
    uint64_t sp;
    uint64_t lr;
    asm volatile(
        "mov x0, sp\n"
        "mov %[sp], x0\n"
        "mrs %[lr], elr_el1\n":[sp]"=r"(sp), [lr]"=r"(lr)
    );
    uint64_t val = ((uint64_t)th->sp - sp) / (32 * 9);
    // if(th->sp < sp) {
    //     uart_switch_func(UART_DEFAULT);
    //     uart_send_string("Except Fucked\r\n");
    //     uart_send_u64(th->sp);
    //     uart_send_string("\r\n");
    //     uart_send_u64(sp);
    //     uart_send_string("\r\n");
    //     uart_send_u64(lr);
    //     uart_send_string("\r\n");
    //     uart_switch_func(UART_ASYNC);
    // }
    return val;
}