#include "tmpfs.h"
#include "mm.h"
#include "list.h"
struct file_operations tmpfs_f_ops = {
    .write = &tmpfs_write,
    .read = &tmpfs_read,
    .open = &tmpfs_open,
    .close = &tmpfs_close,
    .lseek64 = &tmpfs_lseek64
};

struct vnode_operations tmpfs_v_ops = {
    .lookup = &tmpfs_lookup,
    .create = &tmpfs_create,
    .mkdir = &tmpfs_mkdir
};

int tmpfs_setup_mount(struct filesystem* fs, struct mount* mount)
{
    if (mount == NULL)
        return 1;
    uart_write_string("tmpfs_setup_mount chk1\n");
    struct vnode *root_node;
    //setup root under mount
    if (mount->root == NULL) {
        root_node = new_vnode(NULL, &tmpfs_v_ops, &tmpfs_f_ops, NULL, DIRECTORY, NULL);
        INIT_LIST_HEAD(&(root_node->internal));
        mount->root = root_node;
        uart_write_string("tmpfs_setup_mount chk1-2\n");
        return 0;
    }
    root_node = mount->root;

    
    uart_write_string("tmpfs_setup_mount chk2\n");
    //setup mount should provide f_ops and v_ops for mount_node.
    
    root_node->f_ops = &tmpfs_f_ops;
    uart_write_string("tmpfs_setup_mount chk2-1\n");
    
    root_node->v_ops = &tmpfs_v_ops;
    uart_write_string("tmpfs_setup_mount chk3\n");
    
    //empty root directory
    INIT_LIST_HEAD(&(root_node->internal));
    uart_write_string("tmpfs_setup_mount chk4\n");
    return 0;
}

static int tmpfs_extend(struct file *file, size_t at_least)
{
    struct tmpfs_file_node *fnode = (struct tmpfs_file_node *)(file->vnode->internal);
    size_t needed_pages = NEEDED_PAGES(at_least);
    char *tmp = alloc_pages(needed_pages);
    if (tmp == NULL) return 1;
    memcpy(tmp, fnode->content_start, fnode->file_size);
    //if fnode->content_start is NULL, free_page does nothing
    free_page(fnode->content_start);
    fnode->content_start = tmp;
    fnode->buffer_size = needed_pages * PAGE_SIZE;
    return 0;
}
extern void ls_dir(struct vnode *dir_node);
int tmpfs_write(struct file* file, const void* buf, size_t len)
{
    // ls_dir(file->vnode->parent);
    //write to f_pos
    //treat internal as a struct tmpfs_file_node *fnode
    struct tmpfs_file_node *fnode = (struct tmpfs_file_node *)(file->vnode->internal);
    
    //copy to large page frames if run out of space for new content
    if (file->f_pos + len > fnode->buffer_size) {
        tmpfs_extend(file, file->f_pos + len);
    }
    char *content_start = fnode->content_start;
    //copy...
    memcpy(content_start + file->f_pos, buf, len);
    //updates f_pos and size after write
    file->f_pos += len;
    fnode->file_size = max(fnode->file_size, file->f_pos);
    // ls_dir(file->vnode->parent);
    return (int)len;
}
int tmpfs_read(struct file* file, void* buf, size_t len)
{
    //read from f_pos
    struct tmpfs_file_node *fnode = (struct tmpfs_file_node *)(file->vnode->internal);
    char *content_start = fnode->content_start;
    char *p = content_start + file->f_pos;
    len = min(len, fnode->file_size - file->f_pos);
    memcpy(buf, p, len);
    //updates f_pos after read
    file->f_pos += len;
    return len;
}
int tmpfs_open(struct vnode* file_node, struct file** target)
{
    //deal with file
    (*target)->f_ops = &tmpfs_f_ops;
    (*target)->f_pos = 0;
    return 0;
}
int tmpfs_close(struct file* file)
{
    //NO THIS IS DELETE
    //deal with fnode
    // struct tmpfs_file_node *fnode = (struct tmpfs_file_node *)file->vnode->internal;
    // free_page(fnode->content_start);
    // memset(fnode, 0, sizeof(struct tmpfs_file_node));
    // kfree(fnode);

    //deal with file
    memset(file, 0, sizeof(struct file));
    return 0;
}

long tmpfs_get_size(struct file *file)
{
    return ((struct tmpfs_file_node *)(file->vnode->internal))->file_size;
}

long tmpfs_exceed_callback(struct file *file, long new_offset)
{
    struct tmpfs_file_node *fnode = (struct tmpfs_file_node *)file->vnode->internal;
    size_t org_file_size = fnode->file_size;
    long errno = tmpfs_extend(file, new_offset);
    if (errno) return errno;
    
    memset(fnode->content_start + org_file_size, 0, new_offset - org_file_size);
    return 0;
}

long tmpfs_below_callback(struct file *file, long new_offset)
{
    return 0;
}

long tmpfs_lseek64(struct file* file, long offset, int whence)
{
    return default_lseek64(file, offset, whence, tmpfs_get_size, tmpfs_exceed_callback, tmpfs_below_callback);
}

int tmpfs_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name)
{
    return example_lookup(dir_node, target, component_name);
}

int tmpfs_create(struct vnode* dir_node, struct vnode** target, const char* component_name)
{
    int errno;
    //target is set in vfs_create
    // * create a new tmpfs_file_node and set it in *target
    struct tmpfs_file_node *fnode = kmalloc(sizeof(struct tmpfs_file_node));
    if (!fnode) {
        return 1;
    }
    // fnode->content_start = alloc_pages(1);
    fnode->content_start = NULL;//alloc_pages(1);
    fnode->buffer_size = 0;//PAGE_SIZE;
    fnode->file_size = 0;
    fnode->vnode = *target;
    (*target)->internal = fnode;
    
    // * add to parent directory
    struct dentry *file_dent;
    if ((errno = example_append2dir(dir_node, *target, &fnode->file_size, component_name, NULL, &file_dent)) != 0) {
        return errno;
    }

    return 0;
}

int tmpfs_mkdir(struct vnode* dir_node, struct vnode** target, const char* component_name)
{
    return example_mkdir(dir_node, target, component_name);
}