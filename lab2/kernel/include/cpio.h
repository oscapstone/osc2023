#ifndef _CPIO_H_
#define _CPIO_H_

/*
    cpio format : https://www.freebsd.org/cgi/man.cgi?query=cpio&sektion=5
    header,file path,file data,header  ......
    header+file path (padding 4 bytes)
    file data (padding 4 bytes)  (max size 4gb)
*/

#define CPIO_NEWC_HEADER_MAGIC "070701"    // big endian
#define CPIO_DEFAULT_PLACE  (void*) 0x8000000
struct cpio_newc_header
{
    char c_magic[6];            //magic   The string	"070701".
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
    char c_check[8];            //this field is always set to zero by writers and ignored by readers.
};

/* write pathname,data,next header into corresponding parameter*/
int cpio_newc_parse_header(struct cpio_newc_header *this_header_pointer,
        char **pathname, unsigned int *filesize, char **data,
        struct cpio_newc_header **next_header_pointer);

#endif /* _CPIO_H_ */
