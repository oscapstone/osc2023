#ifndef INITRD_H
#define INITRD_H
#define __cpio_start (volatile unsigned char *)0x40000
typedef struct {
  char magic[6];      /* Magic header "070701". */
  char ino[8];        /* "i-node" number. */
  char mode[8];       /* Permisions. */
  char uid[8];        /* User ID. */
  char gid[8];        /* Group ID. */
  char nlink[8];      /* Number of hard links. */
  char mtime[8];      /* Modification time. */
  char filesize[8];   /* File size. */
  char devmajor[8];   /* device major/minor number. */
  char devminor[8];   /* device major/minor number. */
  char r_devmajor[8]; /* device major/minor number. */
  char r_devminor[8]; /* device major/minor number. */
  char namesize[8];   /* Length of filename in bytes. */
  char check[8];      /* Check field, should be all zero. */
} cpio_t;
void initrd_list(void);
void initrd_cat(const char *name);

// Callback function of dts
int initrd_fdt_callback(void *, int);
int initrd_getLo(void);

// Loading program
void *initrd_content_getLo(const char *);
#endif // INITRD_H
