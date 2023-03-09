#include "initrd.h"
#include "uart.h"
#include "str.h"

#define memcmp __builtin_memcmp
/**
 * The value stores in the cpio header
 * is hex value, we need this function 
 * to transform from hex char* to bin
 *
 * S the string of hex numbers.
 * n the size of string.
 */
static int hex2bin(char *s, int n){
	int r = 0;
	while(n-- > 0){
		r = r << 4;
		if(*s >= 'A'){
			r += *s++ - 'A' + 10;
		}
		else if (*s >= 0){
			r += *s++ - '0';	// traslate hex to oct
		}
	}
	return r;
}
	

/**
 * Implement the function `ls` in the cpio archive.
 *
 * Note: The cpio header is defined in the initrd.h
 */
void initrd_list(char* buf){
	uart_puts("mode\t\t");
	uart_puts("size\t");
	uart_puts("namesize\t");
	uart_puts("name\n");
	while(!(memcmp(buf, "070701", 6)) && memcmp(buf + sizeof(cpio_t), "TRAILER!!", 9)){	// test magic number of new ascii 
		cpio_t *header = (cpio_t*)buf;
		int ns = hex2bin(header->namesize, 8);	// Get the size of name
		int fs = hex2bin(header->filesize, 8);	// Get teh size of file content

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

		buf += (sizeof(cpio_t) + ns + fs); 	// Jump to next record
	}
}

/**
 * Show the content of target file
 *
 * name: name of the target file.
 */
void initrd_cat(const char* name, char* buf){
	int ns = 0;
	int fs = 0;
	while(!(memcmp(buf, "070701", 6)) && memcmp(buf + sizeof(cpio_t), "TRAILER!!", 9)){	// test magic number of new ascii 
		cpio_t *header = (cpio_t*)buf;
		ns = hex2bin(header->namesize, 8);	// Get the size of name
		fs = hex2bin(header->filesize, 8);	// Get teh size of file content
		// Find the target file
		if(!(strncmp(buf + sizeof(cpio_t), name, ns - 1)))
			break;
		buf += (sizeof(cpio_t) + ns + fs);	// Jump to next record
	}
	if(fs > 0){
		uart_putsn(buf + sizeof(cpio_t), ns);		// Show File name
		uart_puts("\'s content :\n");
		uart_putsn(buf + sizeof(cpio_t) + ns, fs);	// Show File content
	}
	return;
}

