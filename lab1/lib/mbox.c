#include "mbox.h"

int mbox_call(mail_t* mbox, uint8_t ch) {
    uint32_t addr_channel;

    // 1. Read the status register until the empty flag is not set
    while (mmio_read(MBOX_STATUS) & MBOX_FULL) asm volatile("nop");

    // 2. Write the data (shifted into the uppper 28 bits) combined with
    //    the channel (in the lower four bits) to the write register
    addr_channel = (((uint32_t)(uint64_t)mbox) & 0xFFFFFFF0) | (ch & 0xF);
    mmio_write(MBOX_WRITE, addr_channel);

    // Wait for mbox response
    do {
        while (mmio_read(MBOX_STATUS) & MBOX_EMPTY) asm volatile("nop");
    } while (mmio_read(MBOX_READ) != addr_channel);

    /* check response vaild */
    return mbox->header.code == MBOX_RESPONSE;
}

void get_board_revision(uint32_t* board_revision) {
    mail_t mbox = {
        .header.packet_size = MAIL_PACKET_SIZE,
        .header.code = REQUEST_CODE,
        .body.id = GET_BOARD_REVISION,
        .body.buf_size = MAIL_BUF_SIZE,
        .body.code = TAG_REQUEST_CODE,
        .body.end = END_TAG,
    };

    for (uint32_t i = 0; i < MAIL_BODY_BUF_LEN; i++) mbox.body.buf[i] = 0;

    mbox_call(&mbox, MBOX_CH_PROP);

    *board_revision = mbox.body.buf[0];
}

