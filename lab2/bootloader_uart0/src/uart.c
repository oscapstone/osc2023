#define MMIO_BASE       0x3F000000
#define UART0_DR        ((volatile unsigned int*)(MMIO_BASE+0x00201000))
#define UART0_FR        ((volatile unsigned int*)(MMIO_BASE+0x00201018))
#define UART0_IBRD      ((volatile unsigned int*)(MMIO_BASE+0x00201024))
#define UART0_FBRD      ((volatile unsigned int*)(MMIO_BASE+0x00201028))
#define UART0_LCRH      ((volatile unsigned int*)(MMIO_BASE+0x0020102C))
#define UART0_CR        ((volatile unsigned int*)(MMIO_BASE+0x00201030))
#define UART0_IMSC      ((volatile unsigned int*)(MMIO_BASE+0x00201038))
#define UART0_ICR       ((volatile unsigned int*)(MMIO_BASE+0x00201044))

#define MBOX_REQUEST    0

/* channels */
#define MBOX_CH_POWER   0
#define MBOX_CH_FB      1
#define MBOX_CH_VUART   2
#define MBOX_CH_VCHIQ   3
#define MBOX_CH_LEDS    4
#define MBOX_CH_BTNS    5
#define MBOX_CH_TOUCH   6
#define MBOX_CH_COUNT   7
#define MBOX_CH_PROP    8

/* tags */
#define MBOX_TAG_GETSERIAL      0x10004
#define MBOX_TAG_SETCLKRATE     0x38002
#define MBOX_TAG_LAST           0

#define PBASE 0x3F000000
// use phy addr 0x3f instead of bus addr 0x7e
#define GPFSEL0         ((volatile unsigned int*)(MMIO_BASE+0x00200000))
#define GPFSEL1         ((volatile unsigned int*)(MMIO_BASE+0x00200004))
#define GPFSEL2         ((volatile unsigned int*)(MMIO_BASE+0x00200008))
#define GPFSEL3         ((volatile unsigned int*)(MMIO_BASE+0x0020000C))
#define GPFSEL4         ((volatile unsigned int*)(MMIO_BASE+0x00200010))
#define GPFSEL5         ((volatile unsigned int*)(MMIO_BASE+0x00200014))
#define GPSET0          ((volatile unsigned int*)(MMIO_BASE+0x0020001C))
#define GPSET1          ((volatile unsigned int*)(MMIO_BASE+0x00200020))
#define GPCLR0          ((volatile unsigned int*)(MMIO_BASE+0x00200028))
#define GPLEV0          ((volatile unsigned int*)(MMIO_BASE+0x00200034))
#define GPLEV1          ((volatile unsigned int*)(MMIO_BASE+0x00200038))
#define GPEDS0          ((volatile unsigned int*)(MMIO_BASE+0x00200040))
#define GPEDS1          ((volatile unsigned int*)(MMIO_BASE+0x00200044))
#define GPHEN0          ((volatile unsigned int*)(MMIO_BASE+0x00200064))
#define GPHEN1          ((volatile unsigned int*)(MMIO_BASE+0x00200068))
#define GPPUD           ((volatile unsigned int*)(MMIO_BASE+0x00200094))
#define GPPUDCLK0       ((volatile unsigned int*)(MMIO_BASE+0x00200098))
#define GPPUDCLK1       ((volatile unsigned int*)(MMIO_BASE+0x0020009C))

#define VIDEOCORE_MBOX  (MMIO_BASE+0x0000B880)
#define MBOX_READ       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x0))
#define MBOX_POLL       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x10))
#define MBOX_SENDER     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x14))
#define MBOX_STATUS     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x18))
#define MBOX_CONFIG     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x1C))
#define MBOX_WRITE      ((volatile unsigned int*)(VIDEOCORE_MBOX+0x20))
#define MBOX_RESPONSE   0x80000000
#define MBOX_FULL       0x80000000
#define MBOX_EMPTY      0x40000000
#define MBOX_CH_PROP    8

extern volatile unsigned int mbox[36];

/**
 * initialize UART 1
 */
void uart_init ( void ) {
    register unsigned int r;

    /* initialize UART */
    // See ref page: https://oscapstone.github.io/labs/hardware/uart.html

    // 1. Configure the UART clock frequency by mailbox.
    *UART0_CR = 0;         // turn off UART0

    /* set up clock for consistent divisor values */
    mbox[0] = 9*4;
    mbox[1] = MBOX_REQUEST;
    mbox[2] = MBOX_TAG_SETCLKRATE; // set clock rate
    mbox[3] = 12;
    mbox[4] = 8;
    mbox[5] = 2;           // UART clock
    mbox[6] = 4000000;     // 4Mhz
    mbox[7] = 0;           // clear turbo
    mbox[8] = MBOX_TAG_LAST;

    unsigned int l = (((unsigned int)((unsigned long)&mbox)&~0xF) | (MBOX_CH_PROP&0xF));
    /* wait until we can write to the mailbox */

    // do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_FULL);
    
    /* write the address of our message to the mailbox with channel identifier */
    *MBOX_WRITE = l;
    /* now wait for the response */
    // while(1) {
        /* is there a response? */
        // do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_EMPTY);
    //     /* is it a response to our message? */
    //     if(l == *MBOX_READ)
    //         /* is it a valid successful response? */
    //         mbox[1]==MBOX_RESPONSE;
    // }

    // 2. Enable GPIO (almost same as mini UART).
    r=*GPFSEL1;
    r&=~((7<<12)|(7<<15)); // gpio14, gpio15
    r|=(4<<12)|(4<<15);    // alt5
    *GPFSEL1 = r;
    *GPPUD = 0;            // enable pins 14 and 15
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = (1<<14)|(1<<15);
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = 0;        // flush GPIO setup

    *UART0_ICR = 0x7FF;    // clear interrupts. Question: is it really necessary?
    // 3. Set IBRD and FBRD to configure baud rate.
    *UART0_IBRD = 2;       // 115200 baud
    *UART0_FBRD = 0xB;
    // 4. Set LCRH to configure line control.
    *UART0_LCRH = 0x7<<4;  // 8n1, enable FIFOs
    //5. Set CR to enable UART.
    *UART0_CR = 0x301;     // enable Tx, Rx, UART
    return;
}

/**
 * Display a character
 */
void uart_send(char c) {
    while (*UART0_FR&0x20) {
        asm volatile("nop");
    }
    *UART0_DR=c;
}

/**
 * Receive a character
 */
char uart_getc() {
    while (*UART0_FR&0x10) {
        asm volatile("nop");
    }
    return (char)(*UART0_DR);
}

/**
 * Display a string. The '\r\n' issue is handled in this function
 */
void uart_puts(char *s) {
	for (int i = 0; s[i] != '\0'; i++) {
        if(s[i]=='\n')
            uart_send('\r');
		uart_send((char)s[i]);
	}
}

void uart_hexdump(unsigned int d) {
    unsigned int n;
    int c;
    for(c=4;c>=0;c-=4) {
        // get highest tetrad
        n=(d>>c)&0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n+=n>9?0x37:0x30;
        uart_send(n);
    }
    uart_send(' ');
}

void uart_hex(unsigned int d) {
    unsigned int n;
    int c;
    for(c=28;c>=0;c-=4) {
        // get highest tetrad
        n=(d>>c)&0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n+=n>9?0x37:0x30;
        uart_send(n);
    }
}