#include "fs/tmpfs.h"
#include "fs/vfs.h"
#include "string.h"
#include "malloc.h"

struct file_operations tmpfs_file_operations = {tmpfs_write, tmpfs_read, tmpfs_open, tmpfs_close, vfs_lseek64, tmpfs_getsize};
struct vnode_operations tmpfs_vnode_operations = {tmpfs_lookup,tmpfs_create,tmpfs_mkdir};

int register_tmpfs()
{
    struct filesystem fs;
    fs.name = "tmpfs";
    fs.setup_mount = tmpfs_setup_mount;
    return register_filesystem(&fs);
}

int tmpfs_setup_mount(struct filesystem *fs, struct mount *_mount)
{
    _mount->fs = fs;
    _mount->root = tmpfs_create_vnode(0,dir_t);
    return 0;
}

struct vnode* tmpfs_create_vnode(struct mount* _mount, enum node_type type)
{
    struct vnode *v = kmalloc(sizeof(struct vnode));
    v->f_ops = &tmpfs_file_operations;
    v->v_ops = &tmpfs_vnode_operations;
    v->mount = 0;
    struct tmpfs_inode* inode = kmalloc(sizeof(struct tmpfs_inode));
    memset(inode, 0, sizeof(struct tmpfs_inode));
    inode->type = type;              // file or directory
    inode->data = kmalloc(0x1000);
    v->internal = inode;
    return v;
}

// file operations
int tmpfs_write(struct file *file, const void *buf, size_t len)
{
    // stores the specific file system's internal data structure related to that node.
    struct tmpfs_inode *inode = file->vnode->internal;

    // calculates the correct memory address within the data buffer
    memcpy(inode->data + file->f_pos, buf, len);
    file->f_pos += len;

    // file has grown in size, re-assign datasize
    if(inode->datasize < file->f_pos)inode->datasize = file->f_pos;
    return len;
}

int tmpfs_read(struct file *file, void *buf, size_t len)
{
    struct tmpfs_inode *inode = file->vnode->internal;
    
    // not enough data remaining in the file to satisfy the full read request.
    if(len+file->f_pos > inode->datasize)
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

int tmpfs_open(struct vnode *file_node, struct file **target)
{
    (*target)->vnode = file_node;
    (*target)->f_ops = file_node->f_ops;
    (*target)->f_pos = 0;
    return 0;
}

int tmpfs_close(struct file *file)
{
    kfree(file);
    return 0;
}

// vnode operations
int tmpfs_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name)
{
    struct tmpfs_inode *dir_inode = dir_node->internal;
    int child_idx = 0;
    for (; child_idx < MAX_DIR_ENTRY; child_idx++)
    {
        struct vnode *vnode = dir_inode->entry[child_idx];
        if(!vnode)break;
        struct tmpfs_inode *inode = vnode->internal;
        if (strcmp(component_name, inode->name) == 0)
        {
            *target = vnode;
            return 0;
        }
    }

    uart_printf("tmpfs lookup not found\r\n");
    return -1;
}

// file ops
int tmpfs_create(struct vnode *dir_node, struct vnode **target, const char *component_name)
{
    struct tmpfs_inode *inode = dir_node->internal;
    if(inode->type!=dir_t)
    {
        uart_printf("tmpfs create not dir_t\r\n");
        return -1;
    }

    int child_idx = 0;
    for (; child_idx < MAX_DIR_ENTRY; child_idx++)
    {
        //  found an available slot to create the new file
        if (!inode->entry[child_idx])break;

        struct tmpfs_inode *child_inode = inode->entry[child_idx]->internal;
        // already exists in the directory.
        if (strcmp(child_inode->name,component_name)==0)
        {
            uart_printf("tmpfs create file exists\r\n");
            return -1;
        }
    }

    if (child_idx == MAX_DIR_ENTRY)
    {
        uart_printf("DIR ENTRY FULL\r\n");
        return -1;
    }

    struct vnode *_vnode = tmpfs_create_vnode(0, file_t);
    inode->entry[child_idx] = _vnode;
    if (strlen(component_name) > FILE_NAME_MAX)
    {
        uart_printf("FILE NAME TOO LONG\r\n");
        return -1;
    }

    struct tmpfs_inode *newinode = _vnode->internal;
    strcpy(newinode->name, component_name);

    *target = _vnode;
    return 0;
}

// dir ops
int tmpfs_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name)
{
    struct tmpfs_inode *inode = dir_node->internal;

    if (inode->type != dir_t)
    {
        uart_printf("tmpfs mkdir not dir_t\r\n");
        return -1;
    }

    int child_idx = 0;
    for (; child_idx < MAX_DIR_ENTRY; child_idx++)
    {
        //  found an available slot to create the new dict
        if (!inode->entry[child_idx])
        {
            break;
        }
    }

    if(child_idx == MAX_DIR_ENTRY)
    {
        uart_printf("DIR ENTRY FULL\r\n");
        return -1;
    }

    if (strlen(component_name) > FILE_NAME_MAX)
    {
        uart_printf("FILE NAME TOO　LONG\r\n");
        return -1;
    }

    struct vnode* _vnode = tmpfs_create_vnode(0, dir_t);
    inode->entry[child_idx] = _vnode;

    struct tmpfs_inode *newinode = _vnode->internal;
    strcpy(newinode->name, component_name);

    *target = _vnode;
    return 0;
}

long tmpfs_getsize(struct vnode* vd)
{
    struct tmpfs_inode *inode = vd->internal;
    return inode->datasize;
}