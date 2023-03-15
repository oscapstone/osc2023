#include "fs/cpio.h"
#include "string.h"

#define align4(x) (((unsigned int)(x) + 3U) & (~3U))
#define newc_filename(a) ((char *)(a) + sizeof(struct cpio_newc))

static inline int chr2hex(char c) {
    switch (c) {
    case '0' ... '9':
        return c - '0';
    case 'a' ... 'f':
        return c - 'a' + 10;
    default:
        return CPIO_EHEX;
    }
}

static inline unsigned int str2hex(char * str, int bits) {
    unsigned int hex = 0;
    for (int i = 0; i < bits / 4; ++i) {
        int h = chr2hex(str[i]);
        if (h < 0) {
            break;
        }
        hex = (hex << 4) | (unsigned int)h;
    }
    return hex;
}

static inline unsigned int str2h32(char * str) {
    return str2hex(str, 32);
}

static inline unsigned int cpio_magic(char * m) {
    return str2hex(m, 24);
}

int cpio_open_archive(struct cpio * cpio, char * addr, unsigned int fmt) {
    cpio->addr = addr;
    cpio->fmt = fmt;
    return 0;
}

int cpio_read_archive(struct cpio * cpio) {
    if (cpio->addr == (char *)0) {
        return CPIO_EARCHIVE;
    }
    struct cpio_newc * newc = (struct cpio_newc *)cpio->addr;
    unsigned int magic = cpio_magic(newc->magic);
    if (cpio->fmt != magic) {
        return CPIO_EMAGIC;
    }
    static char * prev = (char *)0x0;
    if (prev != cpio->addr) {
        prev = cpio->addr;
        return 0;
    }
    unsigned int filesize = str2h32(newc->filesize);
    unsigned int namesize = str2h32(newc->namesize);
    unsigned int len = align4(namesize + sizeof(struct cpio_newc)) + align4(filesize);
    cpio->addr += len;
    prev = cpio->addr;
    if (!strcmp(newc_filename(cpio->addr), "TRAILER!!!")) {
        cpio->addr = (char *)0;
        return CPIO_EARCHIVE;
    }
    return 0;
}

int cpio_extract(struct cpio * cpio, struct cpio_file * file) {
    struct cpio_newc * newc = (struct cpio_newc *)cpio->addr;
    file->name = newc_filename(newc);
    file->size = str2h32(newc->filesize);
    file->sof = (char *)newc + align4(str2h32(newc->namesize) + sizeof(struct cpio_newc));
    file->pos = file->sof;
    return 0;
}

int cpio_open(struct cpio_file * cpio_file, struct file * file) {
    file->_f = cpio_file;
    file->read = (int (*)(void *, char *, unsigned int))cpio_read;
    file->write = (int (*)(void *, char *, unsigned int))0;
    return 0;
}

int cpio_read(struct cpio_file * cpio_file, char * buf, unsigned int n) {
    int r = 0;
    while (r < n && cpio_file->pos < cpio_file->sof + cpio_file->size) {
        buf[r++] = *(cpio_file->pos++);
    }
    return r;
}