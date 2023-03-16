#include "cpio.h"
#include "filesystem.h"
#include "uart.h"
#include "string.h"

int cat(char* thefilepath)
{
    char* filepath;
    char* filedata;
    unsigned int filesize;
    struct cpio_newc_header *header_pointer = CPIO_DEFAULT_PLACE;

    while(header_pointer!=0)
    {
        int error = cpio_newc_parse_header(header_pointer,&filepath,&filesize,&filedata,&header_pointer);
        //if parse header error
        if(error)
        {
            uart_puts("error");
            break;
        }

        if(strcmp(thefilepath,filepath)==0)
        {
            uart_printf("%s",filedata);
            break;
        }

        //if this is TRAILER!!! (last of file)
        if(header_pointer==0)uart_printf("cat: %s: No such file or directory\n",thefilepath);
    }
    return 0;
}

//working_dir doesn't implemented
int ls(char* working_dir)
{
    char* filepath;
    char* filedata;
    unsigned int filesize;
    struct cpio_newc_header *header_pointer = CPIO_DEFAULT_PLACE;

    while(header_pointer!=0)
    {
        int error = cpio_newc_parse_header(header_pointer,&filepath,&filesize,&filedata,&header_pointer);
        //if parse header error
        if(error)
        {
            uart_puts("error");
            break;
        }

        //if this is not TRAILER!!! (last of file)
        if(header_pointer!=0)uart_printf("%s\n",filepath);
    }
    return 0;
}