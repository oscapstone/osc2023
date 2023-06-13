#include "filesystem/vfs.h"
#include "filesystem/dev_uart.h"
#include "uart.h"
#include "malloc.h"

struct file_operations dev_file_operations = {dev_uart_write, dev_uart_read, dev_uart_open, dev_uart_close, (void *)op_deny, (void *)op_deny};

int init_dev_uart()
{
    return register_dev(&dev_file_operations);
}

int dev_uart_write(struct file *file, const void *buf, size_t len)
{
    const char *cbuf = buf;
    for (int i = 0; i < len; i++)
        uart_async_putc(cbuf[i]);
    return len;
}

int dev_uart_read(struct file *file, void *buf, size_t len)
{
    char *cbuf = buf;
    for (int i = 0; i < len; i++)
        cbuf[i] = uart_async_getc();
    return len;
}

int dev_uart_open(struct vnode *file_node, struct file **target)
{
    // set up file handle
    (*target)->vnode = file_node;
    (*target)->f_ops = &dev_file_operations;
    return 0;
}

int dev_uart_close(struct file *file)
{
    // free file handle
    free(file);
    return 0;
}