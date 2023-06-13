#include "devfs.h"
#include "tmpfs.h"
#include "stdlib.h"
#include "page_alloc.h"
#include "mbox.h"

#define DEVFS_UART_NAME "uart"
#define DEVFS_FRAMEBUFFER_NAME "framebuffer"
#define SEEK_SET 0

filesystem_t devfs = {.name = "devfs", .setup_mount = devfs_setup_mount};
file_operations_t devfs_fops = {.write = devfs_write, .read = devfs_read, .open = devfs_open, .close = devfs_close, .lseek64 = devfs_lseek};
vnode_operations_t devfs_vops = {.lookup = devfs_lookup, .create = devfs_create, .mkdir = devfs_mkdir};

int devfs_setup_mount(struct filesystem *fs, struct mount *mount)
{
    mount->root->f_ops = &devfs_fops;
    mount->root->v_ops = &devfs_vops;
    vnode_t *dir_node = mount->root;

    // Create uart
    vnode_t *node_new = NULL;
    int ret = dir_node->v_ops->create(dir_node, &node_new, DEVFS_UART_NAME);
    if (ret == 0)
        node_new->internal->type = FILE;
    else
        printf("Error, devfs_setup_mount(), failed to create uart, ret=%d\r\n", ret);

    // Create and init framebuffer
    node_new = NULL;
    ret = dir_node->v_ops->create(dir_node, &node_new, DEVFS_FRAMEBUFFER_NAME);
    if (ret == 0)
    {
        node_new->internal->type = FILE;
        node_new->internal->data = framebuffer_init();
    }
    else
        printf("Error, devfs_setup_mount(), failed to create framebuffer, ret=%d\r\n", ret);

    return 0;
}

// fops
int devfs_write(struct file *file, void *buf, size_t len)
{
    if (strcmp(file->vnode->internal->name, DEVFS_UART_NAME) == 0)
    {
        const char *ptr = buf;
        for (size_t i = 0; i < len; i++)
            uart_send_byte(*ptr++);
        return len;
    }
    else if (strcmp(file->vnode->internal->name, DEVFS_FRAMEBUFFER_NAME) == 0)
    {
        node_info_t *node_info = file->vnode->internal;
        const char *ptr = buf;
        char *dest = (char *)(node_info->data + file->f_pos);
        for (size_t i = 0; i < len / sizeof(char); i++)
            *dest++ = *ptr++;
        file->f_pos += len;
        // printf("Debug, devfs_write(), f_pos wrote to %ld\r\n", file->f_pos);
        // uint64_t tk = 0;
        // WAIT_TICKS(tk, 1000);
        return len;
    }
    else
    {
        printf("Error, devfs_write(), writing to unrecognized device %s\r\n", file->vnode->internal->name);
        return 0;
    }
}

int devfs_read(struct file *file, void *buf, size_t len)
{
    if (strcmp(file->vnode->internal->name, DEVFS_UART_NAME) == 0)
    {
        char *ptr = buf;
        for (size_t i = 0; i < len; i++)
            *ptr++ = uart_recv_byte();
        return len;
    }
    else
    {
        printf("Error, devfs_read(), reading from unrecognized device %s\r\n", file->vnode->internal->name);
        return 0;
    }
}

int devfs_open(struct vnode *file_node, struct file **target)
{
    return tmpfs_open(file_node, target);
}

int devfs_close(struct file *file)
{
    return tmpfs_close(file);
}

long devfs_lseek(struct file *file, long offset, int whence)
{
    if (whence == SEEK_SET)
    {
        file->f_pos = (size_t)offset;
        // printf("Debug, devfs_lseek(), f_pos set to %ld\r\n", file->f_pos);
        return file->f_pos;
    }
    else
    {
        printf("Error, devfs_lseek(), unknown whence=%d\r\n", whence);
        return -1;
    }
}

// vops
int devfs_mkdir(struct vnode *dir_node, struct vnode **target, char *component_name)
{
    printf("Error, devfs_mkdir(), cannot mkdir with devfs\r\n");
    return 1;
}

int devfs_create(struct vnode *dir_node, struct vnode **target, char *component_name)
{
    return tmpfs_create(dir_node, target, component_name);
}

int devfs_lookup(struct vnode *dir_node, struct vnode **target, char *component_name)
{
    return tmpfs_lookup(dir_node, target, component_name);
}
