#include "filesystem/initramfs.h"
#include "filesystem/vfs.h"
#include "string.h"
#include "malloc.h"
#include "cpio.h"

struct file_operations initramfs_file_operations = {initramfs_write, initramfs_read, initramfs_open, initramfs_close, vfs_lseek64, initramfs_getsize};
struct vnode_operations initramfs_vnode_operations = {initramfs_lookup, initramfs_create, initramfs_mkdir};

int register_initramfs()
{
    struct filesystem fs;
    // assign filesystem name
    fs.name = "initramfs";
    // set mount callback
    fs.setup_mount = initramfs_setup_mount;
    return register_filesystem(&fs);
}

int initramfs_setup_mount(struct filesystem *fs, struct mount *_mount)
{
    // set struct mount 
    // create vnode
    _mount->fs = fs;
    _mount->root = initramfs_create_vnode(0, dir_t);
    struct initramfs_inode *ramdir_inode = _mount->root->internal;

    // add all file in initramfs to filesystem
    char *filepath;
    char *filedata;
    unsigned int filesize;
    struct cpio_newc_header *header_pointer = cpio_start;
    int idx = 0;

    // parse all file
    while (header_pointer != 0)
    {
        int error = cpio_newc_parse_header(header_pointer, &filepath, &filesize, &filedata, &header_pointer);
        // if parse header error
        if (error)
        {
            uart_printf("%s", "error\n");
            break;
        }

        // if this is not TRAILER!!! (last of file)
        if (header_pointer != 0)
        {
            // only support file (no dir)
            struct vnode *filevnode = initramfs_create_vnode(0, file_t);
            struct initramfs_inode *fileinode = filevnode->internal;
            fileinode->data = filedata;
            fileinode->datasize = filesize;
            fileinode->name = filepath;
            ramdir_inode->entry[idx++] = filevnode;
        }
    }

    return 0;
}

struct vnode *initramfs_create_vnode(struct mount *_mount, enum node_type type)
{
    // creat root vnode
    struct vnode *v = malloc(sizeof(struct vnode));
    v->f_ops = &initramfs_file_operations;
    v->v_ops = &initramfs_vnode_operations;
    v->mount = _mount;
    // create inode
    struct initramfs_inode *inode = malloc(sizeof(struct initramfs_inode));
    memset(inode, 0, sizeof(struct initramfs_inode));
    inode->type = type;
    // no create and write 
    // we dont need ot malloc
    inode->data = malloc(0x1000);
    // point to vnode
    v->internal = inode;
    return v;
}

// file operations
int initramfs_write(struct file *file, const void *buf, size_t len)
{
    // we cannot write on initramfs
    return -1;
}

int initramfs_read(struct file *file, void *buf, size_t len)
{
    // get inode
    struct initramfs_inode *inode = file->vnode->internal;

    // check read out of bound
    if (len + file->f_pos > inode->datasize)
    {
        // resize read length
        len = inode->datasize - file->f_pos;
        // copy to buffer
        memcpy(buf, inode->data + file->f_pos, inode->datasize - file->f_pos);
        file->f_pos += inode->datasize - file->f_pos;
        return inode->datasize - file->f_pos;
    }
    else
    {
        memcpy(buf, inode->data + file->f_pos, len);
        file->f_pos += len;
        return len;
    }
    return -1;
}

int initramfs_open(struct vnode *file_node, struct file **target)
{
    // setup file handle
    // file handle is created in vfs open
    (*target)->vnode = file_node;
    (*target)->f_ops = file_node->f_ops;
    (*target)->f_pos = 0;
    return 0;
}

int initramfs_close(struct file *file)
{
    // free the file handle
    free(file);
    return 0;
}

// vnode operations
int initramfs_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name)
{
    // get inode
    struct initramfs_inode *dir_inode = dir_node->internal;
    int child_idx = 0;

    // iterate over child 
    for (; child_idx < INITRAMFS_MAX_DIR_ENTRY; child_idx++)
    {
        struct vnode *vnode = dir_inode->entry[child_idx];
        if (!vnode)
            break;
        struct initramfs_inode *inode = vnode->internal;
        // find target file
        if (strcmp(component_name, inode->name) == 0)
        {
            *target = vnode;
            return 0;
        }
    }

    uart_printf("initramfs lookup not found\n");
    return -1;
}

int initramfs_create(struct vnode *dir_node, struct vnode **target, const char *component_name)
{
    return -1;
}

int initramfs_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name)
{
    return -1;
}

long initramfs_getsize(struct vnode *vd)
{
    struct initramfs_inode *inode = vd->internal;
    // return inode datasize
    return inode->datasize;
}