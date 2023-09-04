#include <uartfs.h>
#include <mm.h>
#include <mini_uart.h>

static struct filesystem uartfs = {
    .name = "uartfs",
    .mount = uartfs_mount,
    .sync = uartfs_sync
};

static struct vnode_operations uartfs_v_ops = {
    .lookup = uartfs_lookup,
    .create = uartfs_create,
    .mkdir = uartfs_mkdir,
    .isdir = uartfs_isdir,
    .getname = uartfs_getname,
    .getsize = uartfs_getsize
};

static struct file_operations uartfs_f_ops = {
    .write = uartfs_write,
    .read = uartfs_read,
    .open = uartfs_open,
    .close = uartfs_close,
    .lseek64 = uartfs_lseek64,
    .ioctl = uartfs_ioctl
};

/* filesystem methods */

int uartfs_mount(struct filesystem *fs, struct mount *mount){
    struct vnode *oldnode;
    struct uartfs_internal *internal;
    const char *name;

    internal = kmalloc(sizeof(struct uartfs_internal));

    oldnode = mount->root;

    oldnode->v_ops->getname(oldnode, &name);

    internal->name = name;
    internal->oldnode.mount = oldnode->mount;
    internal->oldnode.v_ops = oldnode->v_ops;
    internal->oldnode.f_ops = oldnode->f_ops;
    internal->oldnode.parent = oldnode->parent;
    internal->oldnode.internal = oldnode->internal;

    oldnode->mount = mount;
    oldnode->v_ops = &uartfs_v_ops;
    oldnode->f_ops = &uartfs_f_ops;
    oldnode->internal = internal;

    return 0;
}

/* vnode_operations methods */

int uartfs_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name){
    return -1;
}

int uartfs_create(struct vnode *dir_node, struct vnode **target, const char *component_name){
    return -1;
}

int uartfs_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name){
    return -1;
}

int uartfs_isdir(struct vnode *dir_node){
    return 0;
}

int uartfs_getname(struct vnode *dir_node, const char **name){
    struct uartfs_internal *internal;

    internal = dir_node->internal;

    *name = internal->name;

    return 0;
}

int uartfs_getsize(struct vnode *dir_node){
    return -1;
}

/* file_operations methods */

int uartfs_write(struct file *file, const void *buf, size_t len){
    uart_sendn(buf, len);

    return len;
}

int uartfs_read(struct file *file, void *buf, size_t len){
    uart_recvn(buf, len);

    return len;
}

int uartfs_open(struct vnode *file_node, struct file *target){
    target->vnode = file_node;
    target->f_pos = 0;
    target->f_ops = file_node->f_ops;

    return 0;
}

int uartfs_close(struct file *file){
    file->vnode = NULL;
    file->f_pos = 0;
    file->f_ops = NULL;

    return 0;
}

long uartfs_lseek64(struct file *file, long offset, int whence){
    return -1;
}

int uartfs_ioctl(struct file *file, uint64 request, va_list args){
    return -1;
}

int uartfs_sync(struct filesystem *fs){
    return 0;
}

/* Others */

struct filesystem *uartfs_init(void){
    return &uartfs;
}