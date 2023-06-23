#ifndef _CPIO_H
#define _CPIO_H

#include <type.h>
struct cpio_newc_header {
		   char	   c_magic[6];
		   char	   c_ino[8];
		   char	   c_mode[8];
		   char	   c_uid[8];
		   char	   c_gid[8];
		   char	   c_nlink[8];
		   char	   c_mtime[8];
		   char	   c_filesize[8];
		   char	   c_devmajor[8];
		   char	   c_devminor[8];
		   char	   c_rdevmajor[8];
		   char	   c_rdevminor[8];
		   char	   c_namesize[8];
		   char	   c_check[8];
};

void cpio_ls(char *cpio);
void cpio_cat(char *cpio, char *filename);

/*	
 * Allocate a memory chunk and load the @filename program on it.
 * @output_data point to a pointer to the memory address.
 * Return the size of the file, return 0 if no such file.
 */
uint32 cpio_load_prog(char *cpio, const char *filename, char **output_data);
uint32 cpio_read_hex(char *p);

#endif