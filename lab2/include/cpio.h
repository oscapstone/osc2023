#ifndef CPIO_H
#define CPIO_H

// #define CPIO_DEFAULT_PLACE 0x20000000
#define CPIO_NEWC_HEADER_MAGIC "070701"    // big endian constant, to check whether it is big endian or little endian


struct cpio_newc_header{
  char c_magic[6];        // magic cookie
  char c_ino[8];		    // inode number
  char c_mode[8];		    // file type/access
  char c_uid[8];		    // owners uid
  char c_gid[8];		    // owners gid
  char c_nlink[8];        // # of links at archive creation
  char c_mtime[8];	    // modification time
  char c_filesize[8];	    // length of file in bytes
  char c_devmajor[8];	    // block/char major #
  char c_devminor[8];	    // block/char minor #
  char c_rdevmajor[8];    // special file major #
  char c_rdevminor[8];    // special file minor #
  char c_namesize[8];	    // length of pathname
  char c_check[8];       // 0 OR CRC of bytes of FILE data
} ;

int cpio_newc_parse_header(struct cpio_newc_header *this_header_pointer,
        char **pathname, unsigned int *filesize, char **data,
        struct cpio_newc_header **next_header_pointer);
#endif