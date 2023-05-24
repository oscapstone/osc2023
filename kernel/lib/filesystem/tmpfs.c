#include "filesystem/tmpfs.h"
#include "filesystem/vfs.h"
#include "string.h"
#include "malloc.h"

struct file_operations tmpfs_file_operations = {tmpfs_write, tmpfs_read, tmpfs_open, tmpfs_close, vfs_lseek64, tmpfs_getsize};
struct vnode_operations tmpfs_vnode_operations = {tmpfs_lookup, tmpfs_create, tmpfs_mkdir};

int register_tmpfs()
{
    struct filesystem fs;
    // assign filesystem name
    fs.name = "tmpfs";
    // set the mount callback
    fs.setup_mount = tmpfs_setup_mount;
    return register_filesystem(&fs);
}

int tmpfs_setup_mount(struct filesystem *fs, struct mount *_mount)
{
    // set struct mount 
    _mount->fs = fs;
    // create root vnode
    _mount->root = tmpfs_create_vnode(0, dir_t);
    return 0;
}

struct vnode *tmpfs_create_vnode(struct mount *_mount, enum node_type type)
{
    // create vnode 
    struct vnode *v = malloc(sizeof(struct vnode));
    v->f_ops = &tmpfs_file_operations;
    v->v_ops = &tmpfs_vnode_operations;
    v->mount = 0;
    // internal representation may different
    // create inode
    struct tmpfs_inode *inode = malloc(sizeof(struct tmpfs_inode));
    memset(inode, 0, sizeof(struct tmpfs_inode));
    inode->type = type;
    inode->data = malloc(0x1000);
    // use internal to point to it
    v->internal = inode;
    return v;
}

// file operations
int tmpfs_write(struct file *file, const void *buf, size_t len)
{
    // get inode
    struct tmpfs_inode *inode = file->vnode->internal;

    // copy data from buffer
    memcpy(inode->data + file->f_pos, buf, len);
    file->f_pos += len;

    // update data size
    if (inode->datasize < file->f_pos)
        inode->datasize = file->f_pos;
    return len;
}

int tmpfs_read(struct file *file, void *buf, size_t len)
{
    // get inode
    struct tmpfs_inode *inode = file->vnode->internal;

    // check f_pos is out of bound 
    if (len + file->f_pos > inode->datasize)
    {
        // resize read lenght
        len = inode->datasize - file->f_pos;
        // copy data to buffer
        memcpy(buf, inode->data + file->f_pos, len);
        file->f_pos += inode->datasize - file->f_pos;
        return len;
    }
    else
    {   
        // not out of bound
        memcpy(buf, inode->data + file->f_pos, len);
        file->f_pos += len;
        return len;
    }
    return -1;
}

int tmpfs_open(struct vnode *file_node, struct file **target)
{
    // setup the file handler
    // file handle is created in vfs_open
    (*target)->vnode = file_node;
    (*target)->f_ops = file_node->f_ops;
    (*target)->f_pos = 0;
    return 0;
}

int tmpfs_close(struct file *file)
{
    // release file handle
    free(file);
    return 0;
}

// vnode operations
int tmpfs_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name)
{
    // get inode
    struct tmpfs_inode *dir_inode = dir_node->internal;
    int child_idx = 0;
    // interate every child node
    for (; child_idx < MAX_DIR_ENTRY; child_idx++)
    {
        struct vnode *vnode = dir_inode->entry[child_idx];
        if (!vnode)
            break;
        struct tmpfs_inode *inode = vnode->internal;
        // find target file
        if (strcmp(component_name, inode->name) == 0)
        {
            *target = vnode;
            return 0;
        }
    }

    uart_printf("tmpfs lookup not found\n");
    return -1;
}

int tmpfs_create(struct vnode *dir_node, struct vnode **target, const char *component_name)
{
    // get inode
    struct tmpfs_inode *inode = dir_node->internal;
    
    // check is file type
    if (inode->type != dir_t)
    {
        uart_printf("tmpfs create not dir_t\n");
        return -1;
    }

    int child_idx = 0;
    // find a valid entry (empty)
    for (; child_idx < MAX_DIR_ENTRY; child_idx++)
    {
        // get valid entry
        if (!inode->entry[child_idx])
            break;

        struct tmpfs_inode *child_inode = inode->entry[child_idx]->internal;
        if (strcmp(child_inode->name, component_name) == 0)
        {
            uart_printf("tmpfs create file exists\n");
            return -1;
        }
    }

    // check dir full or not
    if (child_idx == MAX_DIR_ENTRY)
    {
        uart_printf("DIR ENTRY FULL\n");
        return -1;
    }

    // create a vnode and assign to valid entry found above
    struct vnode *_vnode = tmpfs_create_vnode(0, file_t);
    inode->entry[child_idx] = _vnode;
    if (strlen(component_name) > FILE_NAME_MAX)
    {
        uart_printf("FILE NAME TOO　LONG\n");
        return -1;
    }

    // assign file name 
    struct tmpfs_inode *newinode = _vnode->internal;
    strcpy(newinode->name, component_name);

    *target = _vnode;
    return 0;
}

int tmpfs_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name)
{
    // get inode
    struct tmpfs_inode *inode = dir_node->internal;

    if (inode->type != dir_t)
    {
        uart_printf("tmpfs mkdir not dir_t\n");
        return -1;
    }

    // find valid entry
    int child_idx = 0;
    for (; child_idx < MAX_DIR_ENTRY; child_idx++)
        if (!inode->entry[child_idx])
            break;

    if (child_idx == MAX_DIR_ENTRY)
    {
        uart_printf("DIR ENTRY FULL\n");
        return -1;
    }

    if (strlen(component_name) > FILE_NAME_MAX)
    {
        uart_printf("FILE NAME TOO　LONG\n");
        return -1;
    }
    // create a vnode and assign to valid inode
    struct vnode *_vnode = tmpfs_create_vnode(0, dir_t);
    inode->entry[child_idx] = _vnode;
    
    // assign directory name
    struct tmpfs_inode *newinode = _vnode->internal;
    strcpy(newinode->name, component_name);

    *target = _vnode;
    return 0;
}

long tmpfs_getsize(struct vnode *vd)
{
    struct tmpfs_inode *inode = vd->internal;
    return inode->datasize;
}