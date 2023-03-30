#ifndef FS_CPIO
#define FS_CPIO

#include "fs/file.h"

#define CPIO_NEWC       0x070701U

#define CPIO_EMAGIC     ~(0x0)  /* Bad magic */
#define CPIO_EHEX       ~(0x1)  /* Bad hex char */
#define CPIO_EARCHIVE   ~(0x2)  /* Bad cpio address */

struct cpio {
    char * addr;
    unsigned int fmt;
};

struct cpio_newc {
    char magic[6];
    char ino[8];
    char mode[8];
    char uid[8];
    char gid[8];
    char nlink[8];
    char mtime[8];
    char filesize[8];
    char devmajor[8];
    char devminor[8];
    char rdevmajor[8];
    char rdevminor[8];
    char namesize[8];
    char check[8];
} __attribute__((packed));

struct cpio_file {
    char * name;
    unsigned int size;
    char * sof;
    char * pos;
};

int cpio_open_archive(struct cpio * cpio, char * addr, unsigned int fmt);
int cpio_read_archive(struct cpio * cpio);
int cpio_extract(struct cpio * cpio, struct cpio_file * file);

int cpio_open(struct cpio_file * cpio_file, struct file * file);
int cpio_read(struct cpio_file * cpio_file, char * buf, unsigned int n);

#endif