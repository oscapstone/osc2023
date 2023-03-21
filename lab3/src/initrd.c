#include "initrd.h"
#include "dtb.h"
#include "str.h"
#include "uart.h"
#include <stddef.h>

static void *lo_ramfs = 0x0;

int memcmp(void *s1, void *s2, int n) {
  unsigned char *a = s1, *b = s2;
  while (n-- > 0) {
    if (*a != *b) {
      return *a - *b;
    }
    a++;
    b++;
  }
  return 0;
}
/*************************************************************************
 * The value stores in the cpio header is hex value, we need this function
 * to transform from hex char* to bin
 *
 * s : The string of hex numbers.
 * n : The size of string.
 ***********************************************************************/
static int hex2bin(char *s, int n) {
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
 * Implement the function `ls` in the cpio archive.
 *
 * Note: The cpio header is defined in the initrd.h
 * Note: The padding is important, name should pad to 4-byte with header.
 * 	 And content should to padding to 4-byte.
 */
void initrd_list(void) {
  char *buf = (char *)lo_ramfs;
  uart_puts("mode\t\t");
  uart_puts("size\t");
  uart_puts("namesize\t");
  uart_puts("name\n");
  // uart_putsn(buf, 6);
  while (!(memcmp(buf, "070701", 6)) &&
         memcmp(buf + sizeof(cpio_t), "TRAILER!!",
                9)) { // test magic number of new ascii format
    cpio_t *header = (cpio_t *)buf;
    int ns = hex2bin(header->namesize, 8); // Get the size of name
    int fs = hex2bin(header->filesize, 8); // Get teh size of file content
    int pad_n = (4 - ((sizeof(cpio_t) + ns) % 4)) % 4; // Padding size
    int pad_f = (4 - (fs % 4)) % 4;

    // Print the meta of file
    // Mode
    uart_putsn(header->mode, 8);
    uart_puts("\t");
    // filesize
    uart_puti(fs);
    uart_puts("\t");
    // namesize
    uart_puti(ns);
    uart_puts("\t");
    // file name
    uart_putsn(buf + sizeof(cpio_t), ns);
    uart_puts("\n");

    buf += (sizeof(cpio_t) + ns + fs + pad_n + pad_f); // Jump to next record
  }
  return;
}

/**
 * Show the content of target file
 *
 * name: name of the target file.
 */
void initrd_cat(const char *name) {
  char *buf = (char *)lo_ramfs;
  int ns = 0;
  int fs = 0;
  int pad_n = 0;
  int pad_f = 0;
  uart_puts(name);
  while (!(memcmp(buf, "070701", 6)) &&
         memcmp(buf + sizeof(cpio_t), "TRAILER!!",
                9)) { // test magic number of new ascii
    cpio_t *header = (cpio_t *)buf;
    ns = hex2bin(header->namesize, 8); // Get the size of name
    fs = hex2bin(header->filesize, 8); // Get teh size of file content
    pad_n = (4 - ((sizeof(cpio_t) + ns) % 4)) % 4; // Padding size
    pad_f = (4 - (fs % 4)) % 4;
    // Find the target file
    if (!(strncmp(buf + sizeof(cpio_t), name, ns - 1)))
      break;
    buf += (sizeof(cpio_t) + ns + fs + pad_n + pad_f); // Jump to next record
  }
  if (fs > 0) {
    uart_putsn(buf + sizeof(cpio_t), ns); // Show File name
    uart_puts("\'s content :\n");
    uart_putsn(buf + sizeof(cpio_t) + ns + pad_n, fs); // Show File content
  }
  return;
}

/*********************************************************
 * This is callback function for getting the start * address of the initrd. Please use this function * with `fdt_find_do()`.  *******************************************************/
int initrd_fdt_callback(void *start, int size) {
  if (size != 4) {
    uart_puti(size);
    uart_puts("Size not 4!\n");
    return 1;
  }
  uint32_t t = *((uint32_t *)start);
  lo_ramfs = (void *)(b2l_32(t));
  return 0;
}

/********************************************************
 * Function return the location (address) of the initrd.
 *******************************************************/
int initrd_getLo() { return lo_ramfs; }

/********************************************************
 * Return the start address of the content.
 * Called by lodaer to run the program.
 * *****************************************************/
void* initrd_content_getLo(const char* name) {
  char *buf = (char *)lo_ramfs;
  int ns = 0;
  int fs = 0;
  int pad_n = 0;
  int pad_f = 0;
  //uart_puts(name);
  while (!(memcmp(buf, "070701", 6)) &&
         memcmp(buf + sizeof(cpio_t), "TRAILER!!",
                9)) { // test magic number of new ascii
    cpio_t *header = (cpio_t *)buf;
    ns = hex2bin(header->namesize, 8); // Get the size of name
    fs = hex2bin(header->filesize, 8); // Get teh size of file content
    pad_n = (4 - ((sizeof(cpio_t) + ns) % 4)) % 4; // Padding size
    pad_f = (4 - (fs % 4)) % 4;
    // Find the target file
    if (!(strncmp(buf + sizeof(cpio_t), name, ns - 1)))
      break;
    buf += (sizeof(cpio_t) + ns + fs + pad_n + pad_f); // Jump to next record
  }
  if (fs > 0) {
	  return (void*) (buf + sizeof(cpio_t) + ns + pad_n);
  }
  return NULL;
}

