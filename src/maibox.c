#include "mailbox.h"
#include "uart.h"

// the LSB 4 bits is for channl -> 0xf == 16 in decimal, set the address of buffer be mutiple of 16 leave the last four bits to be zeros.
volatile unsigned int __attribute__((aligned(16))) mailbox[8];


/*
Mail box message flow:
  1.Combine the message address (upper 28 bits) with channel number (lower 4 bits)

  2.Check if Mailbox 0 status register’s full flag is set.

  3.If not, then you can write to Mailbox 1 Read/Write register.

  4.Check if Mailbox 0 status register’s empty flag is set.

  5.If not, then you can read from Mailbox 0 Read/Write register.

  6.Check if the value is the same as you wrote in step 1.
*/
int mailbox_call(){
  unsigned int addr =(((unsigned int)((unsigned long)&mailbox) & ~0xF) | (CHANNEL_8 & 0xF));
  while(1){
    if(*MAILBOX_STATUS != MAILBOX_FULL) break;
  }
  *MAILBOX_WRITE= addr;
  while(1){
    while(1){
      if(*MAILBOX_STATUS != MAILBOX_EMPTY) break;
    }
    
    if(addr == *MAILBOX_READ) return 1;
  }
  
  return 0;
}


void get_board_revision(unsigned int* revision){
  mailbox[0] = 7 * 4;
  mailbox[1] = REQUEST_CODE;
  mailbox[2] = GET_BOARD_REVISION;
  mailbox[3] = 4;
  mailbox[4] = TAG_REQUEST_CODE;
  mailbox[5] = 0;
  mailbox[6] = END_TAG;

  mailbox_call();
  
  *revision = mailbox[5];
}

void get_arm_memory(unsigned int* base, unsigned int* size){
  mailbox[0] = 8 * 4;
  mailbox[1] = REQUEST_CODE;
  mailbox[2] = GET_ARM_MEMORY;
  mailbox[3] = 8;
  mailbox[4] = TAG_REQUEST_CODE;
  mailbox[5] = 0;
  mailbox[6] = 0;
  mailbox[7] = END_TAG;

  mailbox_call();

  *base = mailbox[5];
  *size = mailbox[6];
}


