#include "peripherals/mini_uart.h"
#include "peripherals/irq.h"
#include "stdlib.h"

/**
 * common exception handler
 */
void exc_handler(unsigned long type, unsigned long esr, unsigned long elr, unsigned long spsr, unsigned long far)
{
    // print out interruption type
    switch (type)
    {
    case 0:
        uart_send_string("Synchronous");
        break;
    case 1:
        uart_send_string("IRQ");
        break;
    case 2:
        uart_send_string("FIQ");
        break;
    case 3:
        uart_send_string("SError");
        break;
    }
    uart_send_string(": ");
    // decode exception type (some, not all. See ARM DDI0487B_b chapter D10.2.28)
    switch (esr >> 26)
    {
    case 0b000000:
        uart_send_string("Unknown");
        break;
    case 0b000001:
        uart_send_string("Trapped WFI/WFE");
        break;
    case 0b001110:
        uart_send_string("Illegal execution");
        break;
    case 0b010101:
        uart_send_string("System call");
        break;
    case 0b100000:
        uart_send_string("Instruction abort, lower EL");
        break;
    case 0b100001:
        uart_send_string("Instruction abort, same EL");
        break;
    case 0b100010:
        uart_send_string("Instruction alignment fault");
        break;
    case 0b100100:
        uart_send_string("Data abort, lower EL");
        break;
    case 0b100101:
        uart_send_string("Data abort, same EL");
        break;
    case 0b100110:
        uart_send_string("Stack alignment fault");
        break;
    case 0b101100:
        uart_send_string("Floating point");
        break;
    default:
        uart_send_string("Unknown");
        break;
    }
    // decode data abort cause
    if (esr >> 26 == 0b100100 || esr >> 26 == 0b100101)
    {
        uart_send_string(", ");
        switch ((esr >> 2) & 0x3)
        {
        case 0:
            uart_send_string("Address size fault");
            break;
        case 1:
            uart_send_string("Translation fault");
            break;
        case 2:
            uart_send_string("Access flag fault");
            break;
        case 3:
            uart_send_string("Permission fault");
            break;
        }
        switch (esr & 0x3)
        {
        case 0:
            uart_send_string(" at level 0");
            break;
        case 1:
            uart_send_string(" at level 1");
            break;
        case 2:
            uart_send_string(" at level 2");
            break;
        case 3:
            uart_send_string(" at level 3");
            break;
        }
    }
    // dump registers
    uart_send_string("\nSPSR_EL1 ");
    uart_hex(spsr >> 32);
    uart_hex(spsr);
    uart_send_string(" ; ELR_EL1 ");
    uart_hex(elr >> 32);
    uart_hex(elr);
    uart_send_string(" ; ESR_EL1 ");
    uart_hex(esr >> 32);
    uart_hex(esr);
    uart_send_string(" ; FAR_EL1 ");
    uart_hex(far >> 32);
    uart_hex(far);
    uart_send_string("\n");

    return;
}

void el1_irq_interrupt_handler()
{
    // GPU IRQ 57 : UART Interrupt
    if (get32(IRQ_BASIC_PENDING & (1 << 19)))
    {
        if (get32(AUX_MU_IIR_REG) & 0b100) // Receiver holds valid byte
        {
            uart_rx_handler();
        }
        else if (get32(AUX_MU_IIR_REG) & 0b010) // Transmit holding register empty
        {
            uart_tx_handler();
        }
    }
    // ARM Core Timer Interrupt
    else if (get32(CORE0_INTR_SRC) & (1 << 1))
    {
        // arm_core_timer_intr_handler();
    }
    // ARM Local Timer Interrupt
    else if (get32(CORE0_INTR_SRC) & (1 << 11))
    {
        // arm_local_timer_intr_handler();
    }

    return;
}