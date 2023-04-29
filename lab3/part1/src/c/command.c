#include "uart.h"
#include "string.h"
#include "cpio.h"
#include "printf.h"
#include "mm.h"

int user_timer = 0;

void input_buffer_overflow_message(char cmd[])
{
    printf("Follow command: \"");
    printf(cmd);
    printf("\"... is too long to process.\n");

    printf("The maximum length of input is 64.");
}

void command_help()
{
    printf("\n");
    printf("Valid Command:\n");
    printf("\thelp:\t\tprint this help.\n");
    printf("\thello:\t\tprint \"Hello World!\".\n");
    printf("\treboot:\t\treboot Raspberry Pi.\n");
    printf("\tel:\t\tshow exception level.\n");
    printf("\tls:\t\tread initramfs.cpio on the SD card.\n");
    printf("\tcat:\t\tSearch for a specific file.\n");
    printf("\trun:\t\tRun an executable.\n");
    printf("\ttimer_on:\tturn on the core timer.\n");
    printf("\ttimer_off:\tturn off the core timer.\n");
    printf("\tset_timeout:\tset a user timeout.\n");
    printf("\n");
}

void command_hello()
{
    printf("Hello World!\n");
}
void command_show_el(){
    printf("Current EL is:");
    unsigned long el;
    asm volatile("mrs %0, CurrentEL" : "=r"(el));
    printf("%d\n", (unsigned int)el >> 2);
}
void command_ls()
{
    cpio_ls();
}

void command_cat()
{
    char file_name[100];

    printf("file name: ");
    uart_getline(file_name);

    cpio_find_file(file_name);
}

void command_run()
{
    char file_name[100];

    printf("file name: ");
    uart_getline(file_name);

    cpio_run_executable(file_name);
}

void command_timer_on()
{
    asm volatile("svc 1");
}

void command_timer_off()
{
    asm volatile("svc 2");
}

void command_set_timeout()
{
    user_timer = 1;

    char second_string[10];
    printf("time: ");
    uart_getline(second_string);
    int second = atoi(second_string);

    char message[100];
    printf("message: ");
    uart_getline(message);

    asm volatile(
        "mov x10, %0    \n\t"
        "mov x11, %1    \n\t"
        :
        : "r"(&second), "r"(message));

    asm volatile("svc 3");
}

void command_not_found(char *s)
{
    printf("Err: command ");
    printf(s);
    printf(" not found, try <help>\n");
}

void command_reboot()
{
    printf("Start Rebooting...\n");

    *PM_WDOG = PM_PASSWORD | 100;
    *PM_RSTC = PM_PASSWORD | 0x20;

    // while(1);
}