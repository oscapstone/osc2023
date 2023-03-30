#include "uart.h"
#include "initrd.h"
#include "string.h"

// add memory compare, gcc has a built-in for that, clang needs implementation
int memcmp(void *s1, void *s2, int n)
{
    unsigned char *a=s1,*b=s2;
    while(n-->0){ if(*a!=*b) { return *a-*b; } a++; b++; }
    return 0;
}

/* cpio New ACSII format */
typedef struct {
	char c_magic[6];
	char c_ino[8];
        char c_mode[8];
	char c_uid[8];
	char c_gid[8];
	char c_nlink[8];
	char c_mtime[8];
	char c_filesize[8];
	char c_devmajor[8];
	char c_devminor[8];
	char c_rdevmajor[8];
	char c_rdevminor[8];
	char c_namesize[8];
	char c_check[8];
} __attribute__((packed)) cpio_t;

/**
 * Helper function to convert ASCII octal number into binary
 * s string
 * n number of digits
 */
int hex2bin(char *s, int n)
{
  int r = 0;
  while (n-- > 0) {
    r = r << 4;
    if (*s >= 'A') {
      r += *s++ - 'A' + 10;
    } else if (*s >= 0) {
      r += *s++ - '0'; // traslate hex to oct
    }
  }
  return r;
}

/**
 * List the contents of an archive
 */
void initrd_ls()
{
    char *buf = (unsigned char *)0x20000000;
    uart_puts("Mode\t\tFile size\t\tName size\tFile name\n");

    // uart_puts(buf);
    // if it's a cpio archive. Cpio also has a trailer entry
    while(!memcmp(buf,"070701",6) && memcmp(buf+sizeof(cpio_t),"TRAILER!!",9)) {
        cpio_t *header = (cpio_t*)buf;
        int ns = hex2bin(header->c_namesize,8);
        int fs = hex2bin(header->c_filesize,8);
	int pad_n = (4 - ((sizeof(cpio_t) + ns) % 4)) % 4; // Padding size
    	int pad_f = (4 - (fs % 4)) % 4;

        // print out meta information
        uart_hex(hex2bin(header->c_mode,8));  // mode (access rights + type)
        uart_send('\t');
        uart_hex((unsigned int)((unsigned long)buf) + sizeof(cpio_t) + ns);
   
        uart_hex(fs);                       // file size in hex
        uart_send('\t');
	uart_hex(ns);                       // name size in hex
        uart_send('\t');

        uart_puts(buf + sizeof(cpio_t));      // filename
        uart_puts("\n");
        // jump to the next file
        buf += (sizeof(cpio_t) + ns + fs + pad_n + pad_f);
    }
}

void initrd_cat(const char *name)
{
    char *buf = (unsigned char *)0x20000000;
    int ns = 0;
    int fs = 0;
    int pad_n = 0; 
    int pad_f = 0;
    while(!memcmp(buf,"070701",6) && memcmp(buf+sizeof(cpio_t),"TRAILER!!",9)) {
	cpio_t *header = (cpio_t *)buf;
    	ns = hex2bin(header->c_namesize, 8); // Get the size of name
    	fs = hex2bin(header->c_filesize, 8); // Get the size of file content
    	pad_n = (4 - ((sizeof(cpio_t) + ns) % 4)) % 4; // Padding size
    	pad_f = (4 - (fs % 4)) % 4;

	if (!(strncmp(buf + sizeof(cpio_t), name, ns - 1)))
		break;
	buf += (sizeof(cpio_t) + ns + fs + pad_n + pad_f); // Jump to next record
    }

    if (fs > 0) {
	uart_puts(buf + sizeof(cpio_t));      // filename
        uart_puts("\n");
	uart_puts(buf + sizeof(cpio_t) + ns + pad_n);      // content
 	uart_puts("\n");
    }
}
