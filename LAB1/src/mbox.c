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

#include "peripherals/gpio.h"
#include "mbox.h"
#include "mini_uart.h"

/* mailbox message buffer */
volatile unsigned int __attribute__((aligned(16))) mbox[36];

#define VIDEOCORE_MBOX (PBASE + 0x0000B880)
#define MBOX_READ ((volatile unsigned int *)(VIDEOCORE_MBOX + 0x0))
#define MBOX_POLL ((volatile unsigned int *)(VIDEOCORE_MBOX + 0x10))
#define MBOX_SENDER ((volatile unsigned int *)(VIDEOCORE_MBOX + 0x14))
#define MBOX_STATUS ((volatile unsigned int *)(VIDEOCORE_MBOX + 0x18))
#define MBOX_CONFIG ((volatile unsigned int *)(VIDEOCORE_MBOX + 0x1C))
#define MBOX_WRITE ((volatile unsigned int *)(VIDEOCORE_MBOX + 0x20))
#define MBOX_RESPONSE 0x80000000
#define MBOX_FULL 0x80000000
#define MBOX_EMPTY 0x40000000

/**
 * Make a mailbox call. Returns 0 on failure, non-zero on success
 */
int mbox_call(unsigned char ch)
{
  unsigned int r = (((unsigned int)((unsigned long)&mbox) & ~0xF) | (ch & 0xF));
  /* wait until we can write to the mailbox */
  do
  {
    asm volatile("nop");
  } while (*MBOX_STATUS & MBOX_FULL);
  /* write the address of our message to the mailbox with channel identifier */
  *MBOX_WRITE = r;
  /* now wait for the response */
  while (1)
  {
    /* is there a response? */
    do
    {
      asm volatile("nop");
    } while (*MBOX_STATUS & MBOX_EMPTY);
    /* is it a response to our message? */
    if (r == *MBOX_READ)
      /* is it a valid successful response? */
      return mbox[1] == MBOX_RESPONSE;
  }
  return 0;
}


void mbox_run(){
    mbox[0] = 17 * 4;		// length of the message
    mbox[1] = MBOX_REQUEST; // this is a request message

    mbox[2] = MBOX_TAGS_GET_BOARD_REVISION; // get revision command
    mbox[3] = 4;				  // buffer size
    mbox[4] = 8;
    // clear output buffer
    mbox[5] = 0;

    mbox[6] = MBOX_TAGS_GET_ARM_MEM; // get ARM mem command
    mbox[7] = 8;		   // buffer size
    mbox[8] = 8;
    // clear output buffer
    mbox[9] = 0;
    mbox[10] = 0;

    mbox[11] = MBOX_TAGS_GET_BOARD_SERIAL; // get serial number command
    mbox[12] = 8;				 // buffer size
    mbox[13] = 8;
    // clear output buffer
    mbox[14] = 0;
    mbox[15] = 0;

    mbox[16] = MBOX_TAGS_END;


    // mbox[0] = 14 * 4;		// length of the message
    // mbox[1] = MBOX_REQUEST; // this is a request message

    // mbox[2] = GET_BOARD_REVISION; // get revision command
    // mbox[3] = 4;				  // buffer size
    // // clear output buffer
    // mbox[4] = 0;

    // mbox[5] = GET_ARM_MEM; // get ARM mem command
    // mbox[6] = 8;		   // buffer size
    // // clear output buffer
    // mbox[7] = 0;
    // mbox[8] = 0;

    // mbox[9] = GET_BOARD_SERIAL; // get serial number command
    // mbox[10] = 8;				// buffer size
    // // clear output buffer
    // mbox[11] = 0;
    // mbox[12] = 0;

    // mbox[13] = END_TAG;

    // send the message to the GPU and receive answer
    if (mbox_call(MBOX_CH_PROP))
    {
      uart_send_string("My board revision is: ");
      uart_hex(mbox[5]);
      uart_send('\n');
      uart_send_string("My ARM memory base address: ");
      uart_hex(mbox[9]);
      uart_send('\n');
      uart_send_string("My ARM memory size: ");
      uart_hex(mbox[10]);
      uart_send('\n');
      uart_send_string("My serial number is: ");
      uart_hex(mbox[15]);
      uart_hex(mbox[14]);
      uart_send('\n');
    }
    else
    {
      uart_send_string("Unable to query serial!\n");
    }
}