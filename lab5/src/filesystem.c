#include "cpio.h"
#include "filesystem.h"
#include "uart.h"
#include "string.h"
#include "sched.h"

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
            for(unsigned int i=0;i<filesize;i++)
            {
                uart_async_putc(filedata[i]);
            }
            break;
        }

        //if this is TRAILER!!! (last of file)
        if(header_pointer==0)uart_printf("cat: %s: No such file or directory\r\n",thefilepath);
    }
    return 0;
}

int execfile(char* thefilepath)
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
            exec_thread(filedata, filesize);
            break;
        }

        //if this is TRAILER!!! (last of file)
        if(header_pointer==0)uart_printf("execfile: %s: No such file or directory\r\n",thefilepath);
    }
    return 0;
}

char* get_file_start(char *thefilepath)
{
    char *filepath;
    char *filedata;
    unsigned int filesize;
    struct cpio_newc_header *header_pointer = CPIO_DEFAULT_PLACE;

    while (header_pointer != 0)
    {
        int error = cpio_newc_parse_header(header_pointer, &filepath, &filesize, &filedata, &header_pointer);
        //if parse header error
        if (error)
        {
            uart_puts("error");
            break;
        }

        if (strcmp(thefilepath, filepath) == 0)
        {
            return filedata;
        }

        //if this is TRAILER!!! (last of file)
        if (header_pointer == 0)
            uart_printf("execfile: %s: No such file or directory\r\n", thefilepath);
    }
    return 0;
}

unsigned int get_file_size(char *thefilepath)
{
    char *filepath;
    char *filedata;
    unsigned int filesize;
    struct cpio_newc_header *header_pointer = CPIO_DEFAULT_PLACE;

    while (header_pointer != 0)
    {
        int error = cpio_newc_parse_header(header_pointer, &filepath, &filesize, &filedata, &header_pointer);
        //if parse header error
        if (error)
        {
            uart_puts("error");
            break;
        }

        if (strcmp(thefilepath, filepath) == 0)
        {
            return filesize;
        }

        //if this is TRAILER!!! (last of file)
        if (header_pointer == 0)
            uart_printf("execfile: %s: No such file or directory\r\n", thefilepath);
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
            uart_printf("%s","error\r\n");
            break;
        }

        //if this is not TRAILER!!! (last of file)
        if(header_pointer!=0)uart_async_printf("%s\r\n",filepath);
    }
    return 0;
}