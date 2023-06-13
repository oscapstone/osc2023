#include "_cpio.h"
#include "utils_c.h"
#include "allocator.h"
#include "mini_uart.h"
#include "timer.h"

#define KSTACK_SIZE 0x2000
#define USTACK_SIZE 0x2000

unsigned int hex2dec(char *s)
{
    unsigned int r = 0;
    for (int i = 0; i < 8; ++i)
    {
        if (s[i] >= '0' && s[i] <= '9')
        {
            r = r * 16 + s[i] - '0';
        }
        else
        {
            r = r * 16 + s[i] - 'A' + 10;
        }
    }
    return r;
}

char *findFile(char *name)
{
    char *addr = cpio_addr;
    while (utils_str_compare((char *)(addr + sizeof(cpio_header)), "TRAILER!!!") != 0)
    {
        if ((utils_str_compare((char *)(addr + sizeof(cpio_header)), name) == 0))
        {
            return addr;
        }
        cpio_header *header = (cpio_header *)addr;
        unsigned int pathname_size = hex2dec(header->c_namesize);
        unsigned int file_size = hex2dec(header->c_filesize);
        unsigned int headerPathname_size = sizeof(cpio_header) + pathname_size;

        align(&headerPathname_size, 4);
        align(&file_size, 4);
        addr += (headerPathname_size + file_size);
    }
    return 0;
}
void cpio_ls()
{
    char *addr = cpio_addr;
    while (utils_str_compare((char *)(addr + sizeof(cpio_header)), "TRAILER!!!") != 0)
    {
        cpio_header *header = (cpio_header *)addr;
        unsigned int pathname_size = hex2dec(header->c_namesize);
        unsigned int file_size = hex2dec(header->c_filesize);
        unsigned int headerPathname_size = sizeof(cpio_header) + pathname_size;

        align(&headerPathname_size, 4);
        align(&file_size, 4);

        uart_send_string(addr + sizeof(cpio_header)); // print the file name
        uart_send_string("\n");

        addr += (headerPathname_size + file_size);
    }
}

void cpio_cat(char *filename)
{
    char *target = findFile(filename);
    if (target)
    {
        cpio_header *header = (cpio_header *)target;
        unsigned int pathname_size = hex2dec(header->c_namesize);
        unsigned int file_size = hex2dec(header->c_filesize);
        unsigned int headerPathname_size = sizeof(cpio_header) + pathname_size;

        align(&headerPathname_size, 4);
        align(&file_size, 4);

        char *file_content = target + headerPathname_size;
        for (unsigned int i = 0; i < file_size; i++)
        {
            uart_send(file_content[i]); // print the file content
        }
        uart_send_string("\n");
    }
    else
    {
        uart_send_string("Not found the file\n");
    }
}

// void cpio_load_program(char *filename)
// {
//     char *exe_format = ".img";
//     char *verify_format = strstr(filename,exe_format);
//     if (utils_str_compare(verify_format,exe_format))
//     {
//         uart_send_string("Only support exe for binary .img file.\n");
//     }
//     else{
//         char *prog_addr = findFile(filename);
//         void * put_addr = (void * )0x200000;
//         // void * put_addr = 0x200000; 會警告！！
//         if (prog_addr)
//         {
//             cpio_header *header = (cpio_header *)prog_addr;
//             unsigned int pathname_size = hex2dec(header->c_namesize);
//             unsigned int file_size = hex2dec(header->c_filesize);
//             unsigned int headerPathname_size = sizeof(cpio_header) + pathname_size;

//             align(&headerPathname_size, 4);
//             align(&file_size, 4);

//             // print the file name
//             uart_send_string("----------------");
//             uart_send_string(prog_addr + sizeof(cpio_header)); 
//             uart_send_string("----------------\n");

//             //將二進制內容讀出來從0x20000開始放，所以linker script裡寫的0x20000不代表檔案就真的會load入0x20000
//             char *file_content = prog_addr + headerPathname_size;
//             unsigned char *target = (unsigned char *)put_addr;
//             while (file_size--)
//             {
//                 *target = *file_content;
//                 target++;
//                 file_content++;
//             }
//             core_timer_enable();
//             // asm volatile("mov x0, 0x340  \n");
//             asm volatile("mov x0, 0x3c0  \n");
//             asm volatile("msr spsr_el1, x0   \n");
//             asm volatile("msr elr_el1, %0    \n" ::"r"(put_addr));
//             asm volatile("msr sp_el0, %0    \n" ::"r"(put_addr + USTACK_SIZE));
//             asm volatile("eret   \n");
//         }
//         else
//         {
//             uart_send_string("Not found the program\n");
//         }
//     }
// }


// 該函數接收文件名作為參數，無返回值
// load 『user-program』到正確位置後，將權限從EL1->EL0
void cpio_load_program(char *filename)
{
    char *exe_format = ".img";
    char *verify_format = strstr(filename,exe_format);
    if (utils_str_compare(verify_format,exe_format))
    {
        uart_send_string("Only support exe for binary .img file.\n");
    }
    else{
        // 查找文件，返回該文件的地址，如果找到了，prog_addr 就不為空
        char *prog_addr = findFile(filename);

        // 定義變量 put_addr，將目標地址設置為 0x200000
        void *put_addr = (void *)0x200000;

        // 如果找到了文件
        if (prog_addr)
        {
            // 將文件頭部的數據轉換成 cpio_header 的類型
            cpio_header *header = (cpio_header *)prog_addr;

            // 獲取文件名稱的大小和文件大小，並將其轉換成 unsigned int 類型
            unsigned int pathname_size = hex2dec(header->c_namesize);
            unsigned int file_size = hex2dec(header->c_filesize);

            // 計算文件頭部的大小，包括文件名稱大小、文件大小等
            unsigned int headerPathname_size = sizeof(cpio_header) + pathname_size;

            // 計算出文件頭部大小和 4 的對齊後的大小
            align(&headerPathname_size, 4);
            align(&file_size, 4);

            // 使用串口打印信息，提示正在加載該文件的文件名
            uart_send_string("----------------");
            uart_send_string(prog_addr + sizeof(cpio_header));
            uart_send_string("----------------\n");

            // 將文件內容從源地址放到目標地址上
            // 將二進制內容讀出來從0x20000開始放，所以linker script裡寫的0x20000不代表檔案就真的會load入0x20000
            char *file_content = prog_addr + headerPathname_size;
            unsigned char *target = (unsigned char *)put_addr;
            while (file_size--)
            {
                *target = *file_content;
                target++;
                file_content++;
            }//在此由低位往高位擺以及相反會怎樣？？

            // 啟用核心計時器
            core_timer_enable();

            //set spsr_el1 to 0x3c0 and elr_el1 to the program’s start address.
            // set the user program’s stack pointer to a proper position by setting sp_el0.
            // issue eret to return to the user code.
            // asm volatile("mov x0, 0x340  \n");
            asm volatile("mov x0, 0x3c0  \n");
            asm volatile("msr spsr_el1, x0   \n");
            asm volatile("msr elr_el1, %0    \n" ::"r"(put_addr));
            asm volatile("msr sp_el0, %0    \n" ::"r"(put_addr + USTACK_SIZE));
            asm volatile("eret   \n");

            // 在這段程式碼中，關鍵字 volatile 的作用是告訴編譯器不要對這些指令進行優化或重排，
            // 以確保它們按照指定的順序執行。
            // 這是因為這些指令是直接操作 CPU 寄存器和系統狀態的，如果出現任何錯誤或異常行為，
            // 可能會導致系統崩潰或不正常運行。
        }
        else
        {
            // 如果沒有找到該文件，使用串口打印信息
            uart_send_string("Not found the program\n");
        }
    }
}

