#include "tmpfs.h"
#include "mm.h"
#include "string.h"
#include "mini_uart.h"


struct vnode_operations *tmpfs_v_ops;
struct file_operations *tmpfs_f_ops;
int tmpfs_registered = 0;

int tmpfs_setup_mount(struct filesystem* fs, struct mount* mount) {

    mount->fs = fs;
    mount->root = tmpfs_new_node(NULL, "/", DIRECTORY);    
    
    printf("[debug] setup root: 0x%x\n", mount);
    printf("[debug] setup root vnode: 0x%x\n", mount->root);

    return 0;
}

int tmpfs_register() {

    if (tmpfs_registered) return -1;
    tmpfs_registered = 1;

    tmpfs_v_ops = (struct vnode_operations *) chunk_alloc(sizeof(struct vnode_operations));
    tmpfs_f_ops = (struct file_operations *) chunk_alloc(sizeof(struct file_operations));

    tmpfs_v_ops->lookup = tmpfs_lookup;
    tmpfs_v_ops->create = tmpfs_create;
    tmpfs_v_ops->mkdir = tmpfs_mkdir;
    tmpfs_v_ops->load_vnode = NULL;

    tmpfs_f_ops->open = tmpfs_open;
    tmpfs_f_ops->read = tmpfs_read;
    tmpfs_f_ops->write = tmpfs_write;
    tmpfs_f_ops->close = tmpfs_close;

    return 0;
}

struct vnode* tmpfs_new_node(struct tmpfs_internal *parent, const char *name, int type) {
    
    struct vnode *new_node = (struct vnode *)chunk_alloc(sizeof(struct vnode));
    struct tmpfs_internal *new_internal = (struct tmpfs_internal *)chunk_alloc(sizeof(struct tmpfs_internal));

    strcpy(new_internal->name, name);
    new_internal->type = type;
    new_internal->parent = parent;
    new_internal->vnode = new_node;
    new_internal->size = 0;
    if (type == REGULAR_FILE)
        new_internal->data = malloc(MAX_FILESIZE);
    else
        new_internal->data = 0;

    if (parent != NULL)
        new_node->parent = parent->vnode;
    new_node->f_ops = tmpfs_f_ops;
    new_node->v_ops = tmpfs_v_ops;
    new_node->mount = 0;
    new_node->internal = (void *)new_internal;

    return new_node;

}

int tmpfs_open(struct vnode* file_node, struct file** target) {
    return SUCCESS;
}

int tmpfs_close(struct file *file) {
    if (file)
        return SUCCESS;
    else 
        return FAIL;
}

int tmpfs_write(struct file *file, const void *buf, unsigned len) {
    struct tmpfs_internal *internal = (struct tmpfs_internal*)file->vnode->internal;
    if (((struct tmpfs_internal *)file->vnode->internal)->type != REGULAR_FILE)
        return FAIL;

    char *dest = &((char *)internal->data)[file->f_pos];
    char *src = (char *)buf;
    int i = 0;
    for (; i < len && internal->size+i < MAX_FILESIZE; i++) {
        dest[i] = src[i];
    }

    internal->size += i;

    return i;
}

int tmpfs_read(struct file *file, void *buf, unsigned len) {
    
    struct tmpfs_internal *internal = (struct tmpfs_internal*)file->vnode->internal;
    if (internal->type != REGULAR_FILE)
        return FAIL;
    
    char *dest = (char*)buf;
    char *src = &((char *)internal->data)[file->f_pos];
    int i = 0;
    for (; i<len && i < internal->size; i++) {
        dest[i] = src[i];
    }
    
    return i;
}

int tmpfs_create(struct vnode* dir_node, struct vnode** target, const char* component_name) {

    struct tmpfs_internal *parent_internal = (struct tmpfs_internal *)dir_node->internal;
    struct vnode *new_node = tmpfs_new_node(parent_internal, component_name, REGULAR_FILE);

    parent_internal->child[parent_internal->size] = (struct tmpfs_internal *)new_node->internal;
    parent_internal->size++;

    *target = new_node;
    return SUCCESS;
}

int tmpfs_mkdir(struct vnode* dir_node, struct vnode** target, const char* component_name) {

    struct tmpfs_internal *parent_internal = (struct tmpfs_internal *)dir_node->internal;
    struct vnode *new_node = tmpfs_new_node(parent_internal, component_name, DIRECTORY);

    parent_internal->child[parent_internal->size] = (struct tmpfs_internal *)new_node->internal;
    parent_internal->size++;

    *target = new_node;
    return SUCCESS;
}

int tmpfs_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name) {
    
    if (stringcmp(component_name, "") == 0) {
        *target = dir_node;
        return 0;
    }
    
    struct tmpfs_internal *internal = (struct tmpfs_internal *)dir_node->internal;

    for (int i=0; i<internal->size; i++) {
        if(stringcmp(internal->child[i]->name, component_name) == 0) {
            *target = internal->child[i]->vnode;
            return internal->child[i]->type;
        }
    }
    
    return FAIL;

}
