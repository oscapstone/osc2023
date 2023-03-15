#include "bcm2835/uart.h"
#include "fs/cpio.h"
#include "fs/file.h"
#include "init/ramfs.h"
#include "string.h"

static inline void _ramfs_cat(struct file * file) {
    char c;
    while (read(file, &c, 1)) {
        uart_send(c);
    }
}

void ramfs_ls() {
    struct cpio cpio;
    cpio_open_archive(&cpio, RAMFS_ADDR, CPIO_NEWC);
    while (cpio_read_archive(&cpio) != CPIO_EARCHIVE) {
        struct cpio_file file;
        cpio_extract(&cpio, &file);
        uart_puts(file.name);
        uart_send('\n');
    }
}

void ramfs_cat(char * filename) {
    struct cpio cpio;
    cpio_open_archive(&cpio, RAMFS_ADDR, CPIO_NEWC);
    while (cpio_read_archive(&cpio) != CPIO_EARCHIVE) {
        struct cpio_file cpio_file;
        cpio_extract(&cpio, &cpio_file);
        if (!strcmp(cpio_file.name, filename)) {
            struct file file;
            cpio_open(&cpio_file, &file);
            return _ramfs_cat(&file);
        }
    }
    uart_puts(filename);
    uart_puts(": file not found\n");
}
