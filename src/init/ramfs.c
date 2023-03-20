#include "bcm2835/uart.h"
#include "fs/cpio.h"
#include "fs/file.h"
#include "init/fdt.h"
#include "init/ramfs.h"
#include "string.h"
#include "utils.h"

static char * _ramfs_addr = 0;

static inline void _ramfs_cat(struct file * file) {
    char c;
    while (read(file, &c, 1)) {
        uart_send(c);
    }
}

static void _callback(struct fdt_node * node) {
    struct fdt_prop * prop = node->prpty;
    while (prop) {
        if (!strcmp(prop->name, "linux,initrd-start")) {
            _ramfs_addr = (char *)(unsigned long long int)load_big32u(prop->val);
            return;
        }
        prop = prop->next;
    }
}

static unsigned int _cmp(struct fdt_node * node) {
    return !strcmp(node->name, "chosen");
}

static inline char * ramfs_addr() {
    if (!_ramfs_addr) {
        fdt_traverse(_callback, _cmp);
    }
    return _ramfs_addr;
}

void ramfs_ls() {
    struct cpio cpio;
    char * addr = ramfs_addr();
    cpio_open_archive(&cpio, addr, CPIO_NEWC);
    while (cpio_read_archive(&cpio) != CPIO_EARCHIVE) {
        struct cpio_file file;
        cpio_extract(&cpio, &file);
        uart_puts(file.name);
        uart_send('\n');
    }
}

void ramfs_cat(char * filename) {
    struct cpio cpio;
    char * addr = ramfs_addr();
    cpio_open_archive(&cpio, addr, CPIO_NEWC);
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
