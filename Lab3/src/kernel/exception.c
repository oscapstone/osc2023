/*
 * Copyright (C) 2018 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include "mini_uart.h"

/**
 * common exception handler
 */
void exception_handler(unsigned long type, unsigned long esr, unsigned long elr, unsigned long spsr, unsigned long far)
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
    uart_send_string(":\n  ESR_EL1 ");
    uart_hex(esr >> 32);
    uart_hex(esr);
    uart_send_string(" ELR_EL1 ");
    uart_hex(elr >> 32);
    uart_hex(elr);
    uart_send_string("\n SPSR_EL1 ");
    uart_hex(spsr >> 32);
    uart_hex(spsr);
    uart_send_string(" FAR_EL1 ");
    uart_hex(far >> 32);
    uart_hex(far);
    uart_send_string("\n");
    // no return from exception for now
    while (1)
        ;
}
