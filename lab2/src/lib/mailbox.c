#include <mailbox.h>

#include <stdint.h>

#include <uart.h>
#include <utils.h>

void mailbox_call(uint32_t* mailbox){
    uint32_t mailbox_addr = (uint32_t)((uint64_t)mailbox & 0xfffffff0);
    mailbox_addr |= 8;
    while((memory_read(MAILBOX_STATUS) & MAILBOX_FULL) != 0);
    memory_write(MAILBOX_WRITE, mailbox_addr);
    while(1){
        while((memory_read(MAILBOX_STATUS) & MAILBOX_EMPTY) != 0);
        uint32_t data = memory_read(MAILBOX_READ);
        if((data & 0xf) == 8) break;
    }
}

void get_board_revision(uint32_t*revision){
    unsigned int mailbox[7];
    mailbox[0] = 7 * 4; // buffer size in bytes
    mailbox[1] = REQUEST_CODE;
    // tags begin
    mailbox[2] = GET_BOARD_REVISION; // tag identifier
    mailbox[3] = 4; // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    // tags end
    mailbox[6] = END_TAG;

    mailbox_call(mailbox); // message passing procedure call, you should implement it following the 6 steps provided above.

    //printf("0x%x\n", mailbox[5]); // it should be 0xa020d3 for rpi3 b+
    *revision = mailbox[5];
}

void get_ARM_memory_base_address_and_size(uint32_t*addr_base, uint32_t*size){
    unsigned int mailbox[8];
    mailbox[0] = 8 * 4; // buffer size in bytes
    mailbox[1] = REQUEST_CODE;
    // tags begin
    mailbox[2] = GET_ARM_MEMORY; // tag identifier
    mailbox[3] = 8; // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    mailbox[6] = 0; // value buffer
    // tags end
    mailbox[7] = END_TAG;

    mailbox_call(mailbox); // message passing procedure call, you should implement it following the 6 steps provided above.

    //printf("0x%x\n", mailbox[5]); // it should be 0xa020d3 for rpi3 b+
    *addr_base = mailbox[5];
    *size = mailbox[6];
}