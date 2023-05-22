#include "fat32.h"
#include "vfs.h"
#include "string.h"
#include "memory.h"
#include "uart1.h"

struct file_operations fat32_file_operations = {fat32_write, fat32_read, fat32_open, fat32_close, fat32_lseek64, fat32_getsize};
struct vnode_operations fat32_vnode_operations = {fat32_lookup, fat32_create, fat32_mkdir};

int register_fat32()
{
    struct filesystem fs;
    fs.name = "fat32";
    fs.setup_mount = fat32_setup_mount;
    return register_filesystem(&fs);
}

int fat32_setup_mount(struct filesystem *fs, struct mount *_mount)
{
    _mount->fs = fs;
    _mount->root = fat32_create_vnode(0, dir_t);
    return 0;
}

struct vnode *fat32_create_vnode(struct mount *_mount, enum fsnode_type type)
{
    struct vnode *v = kmalloc(sizeof(struct vnode));
    v->f_ops = &fat32_file_operations;
    v->v_ops = &fat32_vnode_operations;
    v->mount = 0;
    struct fat32_inode *inode = kmalloc(sizeof(struct fat32_inode));
    memset(inode, 0, sizeof(struct fat32_inode));
    inode->type = type;
    inode->data = kmalloc(0x1000);
    v->internal = inode;
    return v;
}

// file operations
int fat32_write(struct file *file, const void *buf, size_t len)
{
    struct fat32_inode *inode = file->vnode->internal;

    memcpy(inode->data + file->f_pos, buf, len);
    file->f_pos += len;
    if (inode->datasize < file->f_pos)
        inode->datasize = file->f_pos;
    return len;
}

int fat32_read(struct file *file, void *buf, size_t len)
{
    struct fat32_inode *inode = file->vnode->internal;

    if (len + file->f_pos > inode->datasize)
    {
        len = inode->datasize - file->f_pos;
        memcpy(buf, inode->data + file->f_pos, len);
        file->f_pos += inode->datasize - file->f_pos;
        return len;
    }
    else
    {
        memcpy(buf, inode->data + file->f_pos, len);
        file->f_pos += len;
        return len;
    }
    return -1;
}

int fat32_open(struct vnode *file_node, struct file **target)
{
    (*target)->vnode = file_node;
    (*target)->f_ops = file_node->f_ops;
    (*target)->f_pos = 0;
    return 0;
}

int fat32_close(struct file *file)
{
    kfree(file);
    return 0;
}

// vnode operations
int fat32_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name)
{
    struct fat32_inode *dir_inode = dir_node->internal;
    int child_idx = 0;
    for (; child_idx <= MAX_DIR_ENTRY; child_idx++)
    {
        struct vnode *vnode = dir_inode->entry[child_idx];
        if (!vnode)
            break;
        struct fat32_inode *inode = vnode->internal;
        if (strcmp(component_name, inode->name) == 0)
        {
            *target = vnode;
            return 0;
        }
    }

    uart_sendline("fat32 lookup not found\r\n");
    return -1;
}

int fat32_create(struct vnode *dir_node, struct vnode **target, const char *component_name)
{
    struct fat32_inode *inode = dir_node->internal;
    if (inode->type != dir_t)
    {
        uart_sendline("fat32 create not dir_t\r\n");
        return -1;
    }

    int child_idx = 0;
    for (; child_idx <= MAX_DIR_ENTRY; child_idx++)
    {
        if (!inode->entry[child_idx])
            break;

        struct fat32_inode *child_inode = inode->entry[child_idx]->internal;
        if (strcmp(child_inode->name, component_name) == 0)
        {
            uart_sendline("fat32 create file exists\r\n");
            return -1;
        }
    }

    if (child_idx > MAX_DIR_ENTRY)
    {
        uart_sendline("DIR ENTRY FULL\r\n");
        return -1;
    }

    struct vnode *_vnode = fat32_create_vnode(0, file_t);
    inode->entry[child_idx] = _vnode;
    if (strlen(component_name) > MAX_FILE_NAME)
    {
        uart_sendline("FILE NAME TOO LONG\r\n");
        return -1;
    }

    struct fat32_inode *newinode = _vnode->internal;
    strcpy(newinode->name, component_name);

    *target = _vnode;
    return 0;
}

int fat32_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name)
{
    struct fat32_inode *inode = dir_node->internal;

    if (inode->type != dir_t)
    {
        uart_sendline("fat32 mkdir not dir_t\r\n");
        return -1;
    }

    int child_idx = 0;
    for (; child_idx <= MAX_DIR_ENTRY; child_idx++)
    {
        if (!inode->entry[child_idx])
        {
            break;
        }
    }

    if (child_idx > MAX_DIR_ENTRY)
    {
        uart_sendline("DIR ENTRY FULL\r\n");
        return -1;
    }

    if (strlen(component_name) > MAX_FILE_NAME)
    {
        uart_sendline("FILE NAME TOO LONG\r\n");
        return -1;
    }

    struct vnode *_vnode = fat32_create_vnode(0, dir_t);
    inode->entry[child_idx] = _vnode;

    struct fat32_inode *newinode = _vnode->internal;
    strcpy(newinode->name, component_name);

    *target = _vnode;
    return 0;
}

long fat32_lseek64(struct file *file, long offset, int whence)
{
    if (whence == SEEK_SET)
    {
        file->f_pos = offset;
        return file->f_pos;
    }
    return -1;
}

long fat32_getsize(struct vnode *vd)
{
    struct fat32_inode *inode = vd->internal;
    return inode->datasize;
}
