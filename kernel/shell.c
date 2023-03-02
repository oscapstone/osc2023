#include "bcm2835/uart.h"
#include "kernel/shell.h"

#define PM_MGIC 0x5A000000U
#define PM_RSTC PRPHRL(0x10001C)
#define PM_WDOG PRPHRL(0x100024)

#define SBUF_SIZE 0x100

#define sbuf_empty(x) ((x)->i == (x)->o)

struct sbuf {
    unsigned int i;
    unsigned int o;
    char b[SBUF_SIZE];
};

static char strcmp(char * s, char * t) {
    while (*s && *t && *s == *t) {
        ++s; ++t;
    }
    return *s - *t;
}

static void sbuf_push(struct sbuf * buf, char c) {
    buf->b[buf->i] = c;
    buf->i = (buf->i + 1) % SBUF_SIZE;
    if (sbuf_empty(buf)) {
        buf->o = (buf->o + 1) % SBUF_SIZE;
    }
}

static char sbuf_pop(struct sbuf * buf) {
    if (sbuf_empty(buf)) {
        return '\0';
    }
    char c = buf->b[buf->o];
    buf->o = (buf->o + 1) % SBUF_SIZE;
    return c;
}

static void sbuf_getline(struct sbuf * buf, char * line) {
    unsigned int i = 0;
    while (!sbuf_empty(buf)) {
        char c = sbuf_pop(buf);
        if (c == '\n') {
            break;
        }
        line[i++] = c;
    }
    line[i] = '\0';
}

static void shell_exec(char * cmd) {
    if (!strcmp(cmd, "")) {
        return;
    } else if (!strcmp(cmd, "help")) {
        uart_puts("help:    print this help menu\n");
        uart_puts("hello:   print Hello World!\n");
        uart_puts("reboot:  reboot the device\n");
    } else if (!strcmp(cmd, "hello")) {
        uart_puts("Hello World!\n");
    } else if (!strcmp(cmd, "reboot")) {
        uart_puts("rebooting...\n");
        *PM_RSTC = PM_MGIC | 0x20;
        *PM_WDOG = PM_MGIC | 100;
        do {/* wait for rebooting */} while (1);
    } else {
        uart_puts("command not found: ");
        uart_puts(cmd);
        uart_send('\n');
    }
}

void shell_start() {
    struct sbuf buf;
    buf.i = buf.o = 0;

    uart_puts("# ");
    do {
        char c = uart_recv();
        uart_send(c);
        sbuf_push(&buf, c);
        if (c == '\n') {
            char line[SBUF_SIZE];
            sbuf_getline(&buf, line);
            shell_exec(line);
            uart_puts("# ");
        }
    } while (1);
}
