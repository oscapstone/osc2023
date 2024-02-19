#include "stdlib.h"
#include "vfs.h"
#include "tmpfs.h"

int tmpfs_mount(struct filesystem *fs, struct mount *mount)
{
    mount->root = my_malloc(sizeof(vnode_t));
    mount->root->mount = NULL;

    mount->root->v_ops = my_malloc(sizeof(vnode_operations_t));
    mount->root->v_ops->lookup = tmpfs_lookup;
    mount->root->v_ops->create = tmpfs_create;
    mount->root->v_ops->mkdir = tmpfs_mkdir;

    mount->root->f_ops = my_malloc(sizeof(file_operations_t));
    mount->root->f_ops->write = tmpfs_write;
    mount->root->f_ops->read = tmpfs_read;
    mount->root->f_ops->open = tmpfs_open;
    mount->root->f_ops->close = tmpfs_close;

    mount->root->internal = my_malloc(sizeof(node_info_t));
    mount->root->internal->name = my_malloc(strlen("/") + 1);
    strcpy(mount->root->internal->name, "/");

    mount->root->internal->type = DIR;
    mount->root->internal->size = 0;
    mount->root->internal->entry = my_malloc(sizeof(vnode_t *) * MAX_NUM_OF_ENTRY);

    return 0;
}

int tmpfs_lookup(struct vnode *dir_node, struct vnode **target, char *component_name)
{
    if (dir_node->internal->type != DIR)
    {
        printf("'%s' is not a directory.\n", dir_node->internal->name);
        return NOTDIR;
    }

    for (int i = 0; i < dir_node->internal->size; i++)
    {
        if (!strcmp(dir_node->internal->entry[i]->internal->name, component_name))
        {
            *target = dir_node->internal->entry[i];
            return EXISTED;
        }
    }

    // printf("'%s' is not exist. in lookup()\n", component_name);
    return NOTFOUND;
}

int tmpfs_create(struct vnode *dir_node, struct vnode **target, char *component_name)
{
    if (dir_node->internal->size == MAX_NUM_OF_ENTRY)
    {
        printf("'%s' has no more entries.\n", dir_node->internal->name);
        return -1;
    }

    *target = my_malloc(sizeof(vnode_t));
    (*target)->mount = NULL;
    (*target)->v_ops = dir_node->v_ops;
    (*target)->f_ops = dir_node->f_ops;

    (*target)->internal = my_malloc(sizeof(node_info_t));
    (*target)->internal->name = my_malloc(COMPONENT_NAME_MAX);
    strcpy((*target)->internal->name, component_name);
    (*target)->internal->type = FILE;
    (*target)->internal->size = 0;
    (*target)->internal->entry = NULL; // mkdir will my_malloc()
    (*target)->internal->data = NULL;  // open will my_malloc()

    dir_node->internal->entry[dir_node->internal->size] = *target;
    dir_node->internal->size++;

    return 0;
}

int tmpfs_mkdir(struct vnode *dir_node, struct vnode **target, char *component_name)
{
    int ret = tmpfs_create(dir_node, target, component_name);
    if (ret == 0)
    {
        (*target)->internal->type = DIR;
        (*target)->internal->entry = my_malloc(sizeof(vnode_t *) * MAX_NUM_OF_ENTRY);
        return 0;
    }
    else
        return -1;
}

int tmpfs_write(struct file *file, void *buf, size_t len)
{
    size_t writeable_size = TMPFS_FILE_SIZE_MAX - file->f_pos;
    size_t write_len = len <= writeable_size ? len : writeable_size;

    char *buf_ = (char *)buf;
    for (int i = 0; i < write_len; i++)
        file->vnode->internal->data[file->f_pos + i] = buf_[i];

    file->vnode->internal->size += write_len;
    file->f_pos += write_len;

    return write_len;
}

int tmpfs_read(struct file *file, void *buf, size_t len)
{
    size_t readable_size = file->vnode->internal->size - file->f_pos;
    size_t read_len = len <= readable_size ? len : readable_size;

    char *buf_ = (char *)buf;
    for (int i = 0; i < read_len; i++)
        buf_[i] = file->vnode->internal->data[file->f_pos + i];

    file->f_pos += read_len;

    return read_len;
}

int tmpfs_open(struct vnode *file_node, struct file **target)
{
    if (file_node->internal->data == NULL)
        file_node->internal->data = my_malloc(sizeof(char) * TMPFS_FILE_SIZE_MAX);
    *target = my_malloc(sizeof(file_t));
    (*target)->vnode = file_node;
    (*target)->f_pos = 0;
    (*target)->f_ops = file_node->f_ops;

    return 0;
}

int tmpfs_close(struct file *file)
{
    free(file);
    return 0;
}