#include "interrupt.h"
#include "peripherals/mini_uart.h"
#include "peripherals/base.h"
#include "peripherals/mini_uart.h"
#include "utils.h"
#include "test/demo.h"
#include "time.h"
#include "type.h"
#include "event.h"

#define IRQ_BASIC_PENDING (PBASE + 0xB200)
#define IRQ_GPU_PENDING1 (PBASE + 0xB204)
#define IRQ_GPU_PENDING2 (PBASE + 0xB208)
#define CORE_IRQ_SOURCE (0x40000060)
#define GPU_INTERRUPT (0x100)
#define AUX_INT (1 << 29)

int interrupt_counter = 0;
int interrupt_handler_counter = 0;
extern struct k_event demo_timer_event;
int demo_state = 0;

void irqhandler_inc() {
    // asm volatile ("msr DAIFSet, 0xf\n");
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
    interrupt_counter -= 1;
    if(interrupt_counter < 0) interrupt_counter = 0;
    if(interrupt_counter == 0) {
        asm volatile ("msr DAIFClr, 0xf\n");
    }
}
void disable_interrupt() {
    asm volatile ("msr DAIFSet, 0xf\n");
    interrupt_counter += 1;
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
    return;
}

void time_interrupt_handler() {
    struct k_timeout *t;
    uint64_t tick;
    while(1) {
        // uart_send_string("xxxx\r\n");
        asm volatile("mrs %0, cntpct_el0\n":"=r"(tick));
        t = k_timeout_queue_front();
        if(t == NULL) {
            uint64_t x = 0x3f3f3f3fLL;
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
}

void uart_handler() {
    uint32_t pending = (*(volatile unsigned int*)IRQ_GPU_PENDING1);
    if(pending & AUX_INT) {
        unsigned int IER_REG = *(volatile unsigned int*)(AUX_MU_IER_REG);
        if(IER_REG & 1) {
            irqhandler_inc();
            // uart_send_dec(irqhandler_cnt_get());
            uart_int_recv_handler();
        }
        if(IER_REG & 2) {
            uart_int_trans_handler();
        }
    }
}
void spx_irq_handler(unsigned long esr, unsigned long elr, unsigned long spsr) {
    unsigned int pending = read_reg_32(CORE_IRQ_SOURCE);
    if(pending & 0x2) {
        time_interrupt_handler();
    }
    if(pending & GPU_INTERRUPT) {
        uart_handler();
    }
    return;
}

void lower_irq_handler(unsigned long esr, unsigned long elr, unsigned long spsr) {
    unsigned int pending = read_reg_32(CORE_IRQ_SOURCE);
    if(pending & GPU_INTERRUPT) {
        uart_handler();
    }
    if(pending & 0x2) {
        irqhandler_inc();
        demo_state = 1;
        k_event_submit(&demo_timer_event, NULL, 0, 100);
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