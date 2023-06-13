#include "dev.h"
#include "vfs.h"
#include "uart.h"
/////////////////////////////// uart //////////////////////////////

int dev_uart_write(struct file *file, const void *_buf, size_t len)
{
    char *buf = (char *)_buf;
    for (size_t i = 0; i < len; i++) {
        kuart_write(buf[i]);
    }
    return len;
}

int dev_uart_read(struct file *file, void *_buf, size_t len)
{
    char *buf = (char *)_buf;
    for (size_t i = 0; i < len; i++) {
        buf[i] = kuart_read();
    }
    return len;
}

int dev_uart_open(struct vnode *file_node, struct file **target)
{
    struct file *f = *target;
    f->f_ops = &uart_f_ops;
    f->f_pos = 0;
    return 0;
}

int dev_uart_close(struct file *file)
{
    memset(file, 0, sizeof(struct file));
    return 0;
}

long dev_uart_lseek64(struct file *file, long offset, int whence)
{
    return 0;
}

long dev_uart_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name)
{
    //unsupported operation for uart
    return 1;
}

long dev_uart_create(struct vnode* dir_node, struct vnode** target, const char* component_name)
{
    //unsupported operation for uart
    return 1;
}

int dev_uart_mkdir(struct vnode* dir_node, struct vnode** target, const char* component_name)
{
    //unsupported operation for uart
    return 1;
}

struct file_operations uart_f_ops = {
    .write = dev_uart_write,
    .read = dev_uart_read,
    .open = dev_uart_open,
    .close = dev_uart_close,
    .lseek64 = dev_uart_lseek64
};

struct vnode_operations uart_v_ops = {
    .lookup = dev_uart_lookup,
    .create = dev_uart_create,
    .mkdir = dev_uart_mkdir
};

////////////////////////////////// frame buffer //////////////////////////////
int framebuffer_write(struct file *file, const void *_buf, size_t len)
{
    char *start = (char *)(file->vnode->internal) + file->f_pos;
    char *buf = (char *)_buf;
    for (size_t i = 0; i < len; i++) {
        start[i] = buf[i];
    }
    file->f_pos += len;
    return len;
}

int framebuffer_read(struct file *file, void *_buf, size_t len)
{
    //frame buffer is write only
    return 0;
}

int framebuffer_open(struct vnode *file_node, struct file **target)
{
    struct file *f = *target;
    f->f_ops = &framebuffer_f_ops;
    f->f_pos = 0;
    f->flags = FILE_WRITE;
    return 0;
}

int framebuffer_close(struct file *file)
{
    memset(file, 0, sizeof(struct file));
    return 0;
}

long framebuffer_lseek64(struct file *file, long offset, int whence)
{
    if (whence == SEEK_SET) {
        file->f_pos = offset;
    }
    return file->f_pos;
}

struct file_operations framebuffer_f_ops = {
    .write = framebuffer_write,
    .read = framebuffer_read,
    .open = framebuffer_open,
    .close = framebuffer_close,
    .lseek64 = framebuffer_lseek64
};