#include "cpio.h"

char *cpio_start;
char *cpio_end;

/* Parse one file system object */
/* write pathname,data,next header into corresponding parameter */
/* if no next header, next_header_pointer = 0 */
/* return -1 if parse error*/
int cpio_newc_parse_header(struct cpio_newc_header *this_header_pointer, char **pathname, unsigned int *filesize, char **data, struct cpio_newc_header **next_header_pointer)
{
    /* Ensure magic header exists. */
    if (strncmp(this_header_pointer->c_magic, CPIO_NEWC_HEADER_MAGIC, sizeof(this_header_pointer->c_magic)) != 0)
        return -1;

    // transfer big endian 8 byte hex string to unsinged int and store into *filesize
    *filesize = parse_hex_str(this_header_pointer->c_filesize, 8);

    // end of header is the pathname
    *pathname = ((char *)this_header_pointer) + sizeof(struct cpio_newc_header);

    // get file data, file data is just after pathname
    unsigned int pathname_length = parse_hex_str(this_header_pointer->c_namesize, 8);
    unsigned int offset = pathname_length + sizeof(struct cpio_newc_header);
    offset = offset % 4 == 0 ? offset : (offset + 4 - offset % 4); // padding
    *data = (char *)this_header_pointer + offset;

    // get next header pointer
    if (*filesize == 0)
        *next_header_pointer = (struct cpio_newc_header *)*data;
    else
    {
        offset = *filesize;
        *next_header_pointer = (struct cpio_newc_header *)(*data + (offset % 4 == 0 ? offset : (offset + 4 - offset % 4)));
    }

    // if filepath is TRAILER!!! means there is no more files.
    if (strncmp(*pathname, "TRAILER!!!", sizeof("TRAILER!!!")) == 0)
        *next_header_pointer = 0;

    return 0;
}

int cat(char *thefilepath)
{
    char *filepath;
    char *filedata;
    unsigned int filesize;
    struct cpio_newc_header *header_pointer = (struct cpio_newc_header *)cpio_start;

    while (header_pointer)
    {
        int error = cpio_newc_parse_header(header_pointer, &filepath, &filesize, &filedata, &header_pointer);
        // if parse header error
        if (error)
        {
            uart_printf("error\n");
            break;
        }

        if (!strcmp(thefilepath, filepath))
        {
            for (unsigned int i = 0; i < filesize; i++)
                uart_printf("%c", filedata[i]);
            uart_printf("\n");
            break;
        }

        if (header_pointer == 0)
            uart_printf("cat: %s: No such file or directory\r\n", thefilepath);
    }
    return 0;
}

int ls(char *working_dir)
{
    char *filepath;
    char *filedata;
    unsigned int filesize;
    struct cpio_newc_header *header_pointer = (struct cpio_newc_header *)cpio_start;

    while (header_pointer)
    {
        int error = cpio_newc_parse_header(header_pointer, &filepath, &filesize, &filedata, &header_pointer);
        // if parse header error
        if (error)
        {
            uart_printf("error\n");
            break;
        }

        // if this is not TRAILER!!! (last of file)
        if (header_pointer != 0)
            uart_printf("%s\n", filepath);
    }
    return 0;
}