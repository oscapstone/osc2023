#include "dtb.h"
#include "_cpio.h"
#include "mini_uart.h"
#include "utils_c.h"
/*
    It consists of
    a small header +
    the memory reservation block + space(aligned) +
    the structure block + space(aligned) +
    the strings block + space(aligned)
*/
int space = 0;
char *cpio_addr;

// transfer little endian to big endian
uint32_t get_le2be_uint(const void *p)
{
    const unsigned char *bytes = p;
    uint32_t res = bytes[3];
    res |= bytes[2] << 8;
    res |= bytes[1] << 16;
    res |= bytes[0] << 24;
    return res;
}

void send_sapce(int n)
{
    while (n--)
        uart_send_string(" ");
}

// This function takes a callback, a pointer to the beginning of a flattened device tree, a pointer to the beginning of the strings section of the device tree, and the total size of the device tree.
int parse_struct(fdt_callback cb, uintptr_t cur_ptr, uintptr_t strings_ptr, uint32_t totalsize)
{
    uintptr_t end_ptr = cur_ptr + totalsize; // Calculate the end pointer of the device tree.

    // Loop over each section of the device tree.
    while (cur_ptr < end_ptr)
    {
        // Get the next 4-byte token from the device tree.
        uint32_t token = get_le2be_uint((char *)cur_ptr);
        cur_ptr += 4; // Move the current pointer past the token.

        // Determine the type of the token and call the appropriate callback function.
        switch (token)
        {
        case FDT_BEGIN_NODE:
            // Call the callback function with the FDT_BEGIN_NODE token and the name of the node.
            cb(token, (char *)cur_ptr, NULL, 0);
            // Move the current pointer to the end of the node name.
            cur_ptr += align_up(utils_strlen((char *)cur_ptr), 4);
            break;
        case FDT_END_NODE:
            // Call the callback function with the FDT_END_NODE token.
            cb(token, NULL, NULL, 0);
            break;
        case FDT_PROP:
        {
            // Call the callback function with the FDT_PROP token, the name of the property, and the property data.
            uint32_t len = get_le2be_uint((char *)cur_ptr);     // Get the length of the property data.
            cur_ptr += 4;                                       // Move the current pointer past the length field.
            uint32_t nameoff = get_le2be_uint((char *)cur_ptr); // Get the offset of the property name in the strings section.
            cur_ptr += 4;                                       // Move the current pointer past the name offset field.
            cb(token, (char *)(strings_ptr + nameoff), (void *)cur_ptr, len);
            // Move the current pointer past the property data and align it to the next 4-byte boundary.
            cur_ptr += align_up(len, 4);
            break;
        }
        case FDT_NOP:
            // Call the callback function with the FDT_NOP token.
            cb(token, NULL, NULL, 0);
            break;
        case FDT_END:
            // Call the callback function with the FDT_END token and return 0 to indicate successful parsing.
            cb(token, NULL, NULL, 0);
            return 0;
        default:
            // Return -1 to indicate an error.
            return -1;
        }
    }
}
void print_dtb(int type, const char *name, const void *data, uint32_t size)
{
    switch (type)
    {
    // If the type is a beginning of a new node, print the name with indentation
    case FDT_BEGIN_NODE:
        uart_send_string("\n");   // Move to a new line
        send_sapce(space);        // Indent using space variable
        uart_send_string(name);   // Print the name of the node
        uart_send_string("{\n "); // Open the node with curly braces and a new line
        space++;                  // Increase the indentation level
        break;

    // If the type is an end of a node, print the closing curly brace with indentation
    case FDT_END_NODE:
        uart_send_string("\n"); // Move to a new line
        space--;                // Decrease the indentation level
        if (space > 0)          // If there is still indentation, print it
            send_sapce(space);

        uart_send_string("}\n"); // Close the node with a closing curly brace and a new line
        break;

    // If the type is a NOP, do nothing
    case FDT_NOP:
        break;

    // If the type is a property, print the property name with indentation
    case FDT_PROP:
        send_sapce(space);      // Indent using space variable
        uart_send_string(name); // Print the name of the property
        break;

    // If the type is the end of the device tree blob, do nothing
    case FDT_END:
        break;
    }
}

// void print_dtb(int type, const char *name, const void *data, uint32_t size)
// {
//     switch (type)
//     {
//     case FDT_BEGIN_NODE:
//         uart_send_string("\n");
//         send_sapce(space);
//         uart_send_string(name);
//         uart_send_string("{\n ");
//         space++;
//         break;

//     case FDT_END_NODE:
//         uart_send_string("\n");
//         space--;
//         if (space > 0)
//             send_sapce(space);

//         uart_send_string("}\n");
//         break;

//     case FDT_NOP:
//         break;

//     case FDT_PROP:
//         send_sapce(space);
//         uart_send_string(name);
//         break;

//     case FDT_END:
//         break;
//     }
// }
///
void get_initramfs_addr(int type, const char *name, const void *data, uint32_t size)
{
    // 檢查類型是否是 FDT_PROP，並且 name 是否是 "linux,initrd-start"
    if (type == FDT_PROP && !utils_str_compare(name, "linux,initrd-start"))
    {
        // 將 cpio 地址設置為 data 中包含的值
        // 重新加載Ramdisk(cpio)的address,原本是寫在config.txt裡
        cpio_addr = (char *)(uintptr_t)get_le2be_uint(data);

        // 用 UART 打印出 cpio 地址
        uart_send_string("initramfs_addr at ");
        uart_hex((uintptr_t)get_le2be_uint(data));
        uart_send('\n');
    }
}

// 定義一個名為 fdt_traverse 的函數，接收一個回調函數 fdt_callback cb {和一個指向 dtb 的 void 指標 _dtb}，回傳一個整數值。
int fdt_traverse(fdt_callback cb, void *_dtb)
{
    // 將 _dtb 指標轉換為 uintptr_t 型別的指標 dtb_ptr。
    uintptr_t dtb_ptr = (uintptr_t)_dtb;

    // 透過 uart_send_string 函數，印出提示字串，指出 DTB 檔案正在載入，以及它在記憶體中的位址，這個位址是以十六進位表示的。
    uart_send_string("\ndtb loading at:");
    uart_hex(dtb_ptr);
    uart_send('\n');

    // 宣告一個指向 fdt_header 的指標 header，指向 dtb_ptr 這個位址。
    fdt_header *header = (fdt_header *)dtb_ptr;

    // 檢查 DTB 的 header 的 magic 值是否為 0xd00dfeed，如果不是，印出錯誤訊息並回傳 -1。
    if (get_le2be_uint(&(header->magic)) != 0xd00dfeed)
    {
        uart_send_string("header magic != 0xd00dfeed\n");
        return -1;
    }

    // 取得 DTB 的 header 的 totalsize 值，透過函數 get_le2be_uint。
    uint32_t totalsize = get_le2be_uint(&(header->totalsize));

    // 計算出 DTB 的 structure block 的位址，透過函數 get_le2be_uint，以及 DTB header 中的 off_dt_struct 偏移量。
    uintptr_t struct_ptr = dtb_ptr + get_le2be_uint(&(header->off_dt_struct));

    // 計算出 DTB 的 strings block 的位址，透過函數 get_le2be_uint，以及 DTB header 中的 off_dt_strings 偏移量。
    uintptr_t strings_ptr = dtb_ptr + get_le2be_uint(&(header->off_dt_strings));

    // 解析 DTB 的 structure block，透過 parse_struct 函數，並將解析結果透過 cb 回調函數回傳。
    parse_struct(cb, struct_ptr, strings_ptr, totalsize);
}