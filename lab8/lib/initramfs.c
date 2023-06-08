#include "initramfs.h"
#include "mm.h"
#include "string.h"
#include "mini_uart.h"
#include "vfs.h"
#include "string.h"
#include "../include/cpio.h"

struct vnode_operations *initramfs_v_ops;
struct file_operations *initramfs_f_ops;
int initramfs_registered = 0;

int initramfs_setup_mount(struct filesystem* fs, struct mount* mount) {

    mount->fs = fs;
    mount->root = initramfs_new_node(NULL, "/", DIRECTORY);    

    return 0;

}

int initramfs_register() {
    
    if (initramfs_registered) return -1;
    initramfs_registered = 1;

    initramfs_v_ops = (struct vnode_operations *) chunk_alloc(sizeof(struct vnode_operations));
    initramfs_f_ops = (struct file_operations *) chunk_alloc(sizeof(struct file_operations));

    initramfs_v_ops->lookup = initramfs_lookup;
    initramfs_v_ops->create = initramfs_create;
    initramfs_v_ops->mkdir = initramfs_mkdir;
    initramfs_v_ops->load_vnode = NULL;

    initramfs_f_ops->open = initramfs_open;
    initramfs_f_ops->read = initramfs_read;
    initramfs_f_ops->write = initramfs_write;
    initramfs_f_ops->close = initramfs_close;

    return 0;

}

struct vnode* initramfs_new_node(struct initramfs_internal *parent, const char *name, int type) {

    struct vnode *new_node = (struct vnode *)chunk_alloc(sizeof(struct vnode));
    struct initramfs_internal *new_internal = (struct initramfs_internal *)chunk_alloc(sizeof(struct initramfs_internal));

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
    new_node->f_ops = initramfs_f_ops;
    new_node->v_ops = initramfs_v_ops;
    new_node->mount = 0;
    new_node->internal = (void *)new_internal;

    return new_node;

}

int initramfs_open(struct vnode *file_node, struct file **target) {
    return SUCCESS;
}

int initramfs_close(struct file *file) {
    if (file)
        return SUCCESS;
    else 
        return FAIL;
}

int initramfs_write(struct file *file, const void *buf, unsigned len) {
    return FAIL;
}

int initramfs_read(struct file *file, void *buf, unsigned len) {
    
    struct initramfs_internal *internal = (struct initramfs_internal*)file->vnode->internal;
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

int initramfs_create(struct vnode *dir_node, struct vnode **target, const char *component_name) {
    return FAIL;
}

int initramfs_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name) {
    return FAIL;
}

int initramfs_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name) {

    if (stringcmp(component_name, "") == 0) {
        *target = dir_node;
        return 0;
    }
    
    struct initramfs_internal *internal = (struct initramfs_internal *)dir_node->internal;

    for (int i=0; i<internal->size; i++) {
        if(stringcmp(internal->child[i]->name, component_name) == 0) {
            *target = internal->child[i]->vnode;
            return internal->child[i]->type;
        }
    }
    
    return FAIL;

}

void parse_initramfs() {

    vfs_chdir("/initramfs");

    struct cpio_newc_header *header;
    unsigned int filesize;
    unsigned int namesize;
    unsigned int offset;
    unsigned int c_mode;
    void *data;
    char *filename;

    header = DEVTREE_CPIO_BASE;

    while (1) {
        
        struct vnode *target_node;
        char target_path[VFS_PATHMAX];

        filename = ((void*)header) + sizeof(struct cpio_newc_header);
        
        if (stringncmp((char*)header, CPIO_HEADER_MAGIC, 6) != 0) {
            uart_send_string("invalid magic\n");
            break;
        }
        if (stringncmp(filename, CPIO_FOOTER_MAGIC, 11) == 0) break;

        //uart_send_string(filename);
        //uart_send('\n');

        namesize = hexstr_to_uint(header->c_namesize, 8);
        filesize = hexstr_to_uint(header->c_filesize, 8);
        c_mode = hexstr_to_uint(header->c_mode, 5);
        offset = sizeof(struct cpio_newc_header) + namesize;
        if (offset % 4 != 0) 
            offset = ((offset/4) + 1) * 4;
        data = ((void *)header) + offset;

        if (stringncmp(header->c_mode, "00004", 5) == 0) { //dir

            printf("[debug] parse cpio mkdir %s\n", filename);
            if (stringcmp(filename, ".") != 0 && stringcmp(filename, "..") != 0) {
                traverse(filename, &target_node, target_path);
                struct initramfs_internal *parent_internal = (struct initramfs_internal *)target_node->internal;
                struct vnode *new_node = initramfs_new_node(parent_internal, target_path, DIRECTORY);

                parent_internal->child[parent_internal->size] = (struct initramfs_internal *)new_node->internal;
                parent_internal->size++;
            } else {
                printf("[debug] mkdir skipped\n");
            }

        } else if (stringncmp(header->c_mode, "00008", 5) == 0) { //reg file

            printf("[debug] parse cpio create file %s, data at 0x%x with size 0x%d\n", filename, data, filesize);
            traverse(filename, &target_node, target_path);
            struct initramfs_internal *parent_internal = (struct initramfs_internal *)target_node->internal;
            struct vnode *new_node = initramfs_new_node(parent_internal, target_path, REGULAR_FILE);

            parent_internal->child[parent_internal->size] = (struct initramfs_internal *)new_node->internal;
            parent_internal->size++;

            ((struct initramfs_internal *)new_node->internal)->data = data;
            ((struct initramfs_internal *)new_node->internal)->size = filesize;

        } else {
            printf("[error] failed to parse cmode %d\n", c_mode);
        }
        
        if (filesize % 4 != 0)
            filesize = ((filesize/4) + 1) * 4;

        offset = offset + filesize;

        header = ((void*)header) + offset;
        
    }

    vfs_chdir("/");

}
