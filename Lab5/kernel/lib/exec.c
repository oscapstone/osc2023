#include "cpio.h"
#include "malloc.h"
#include "exec.h"
#include "sched.h"

extern char *cpio_start;

void execute(char *program) {
    char *user_stack = simple_malloc(USTACK_SIZE);

    // EL1 to EL0
    asm volatile(
        "msr spsr_el1, xzr\n\t" // Set jump target EL0t and EL0t interrupt open
        "msr elr_el1, %0\n\t"   // Set position to go to in EL0t
        "msr sp_el0, %1\n\t"    // Set stack pointer in EL0t
        "eret\n\t"::"r"(program),
        "r"(user_stack + USTACK_SIZE) // stack pointer -> add size
    );
}

int exec_file(char *thefilepath)
{
    char* filepath;
    char* filedata;
    unsigned int filesize;
    struct cpio_newc_header *header_pointer = (struct cpio_newc_header *)cpio_start;

    while (header_pointer != 0) // find the file to execute
    {
        int error = cpio_newc_parse_header(header_pointer,&filepath,&filesize,&filedata,&header_pointer);
        //if parse header error
        if(error)
        {
            uart_printf("error\n");
            break;
        }

        if(!strcmp(thefilepath, filepath))
        {
            // uart_printf("dbg: thread_exec\n");
            thread_exec(filedata, filesize);
            break;
        }

        //if this is TRAILER!!! (last of file)
        if (header_pointer == 0)
        {
            uart_printf("exec_file: %s: No such file or directory\n", thefilepath);
            return -1;
        }
    }
    return 0;
}

