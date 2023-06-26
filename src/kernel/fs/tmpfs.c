#include "fs/vfs.h"
#include "fs/tmpfs.h"
#include "mem/mem.h"
#include "utils.h"


struct file tmpfs_filehandle[24];
static uint8_t tmpfs_first_mount = 0;

struct filesystem tmpfs = {
    .name = "tmpfs",
    .root = NULL,
    .init_fs = &init_tmpfs,
    .setup_mount = &mount_tmpfs
};

struct vnode_operations tmpfs_vnode_ops = {
    .lookup = &tmpfs_lookup,
    .create = &tmpfs_create,
    .mkdir = &tmpfs_mkdir
};

struct file_operations tmpfs_file_ops = {
    .write = &tmpfs_write,
    .read = &tmpfs_read,
    .open = &tmpfs_open,
    .close = &tmpfs_close,
    .lseek64 = &tmpfs_lseek64,
    .stat = &tmpfs_stat,
    .ioctl = &fops_not_support
};

int init_tmpfs(struct filesystem *fs) {
    fs->root = (struct vnode*)kmalloc(sizeof(struct vnode));
    fs->root->v_ops = &tmpfs_vnode_ops;
    fs->root->f_ops = &tmpfs_file_ops;
    ds_list_head_init(&(fs->root->ch_list));
    ds_list_head_init(&(fs->root->v_head));
    fs->root->flag = VFS_DIR;
    fs->root->mount = NULL;
    fs->root->internal = (struct tmpfs_vinfo*)kmalloc(sizeof(struct tmpfs_vinfo));
    struct tmpfs_vinfo *info = fs->root->internal;
    info->content = NULL;
    strncpy((info->name), "/", 2);
    info->namesize = 1;
    return 0;
}

int mount_tmpfs(struct filesystem *fs, struct mount *mnt) {
    // general version of mount

    mnt->root = fs->root;
    if(fs->root->mount == NULL) {
        fs->root->mount = mnt;
    } 

    mnt->fs = fs;
    if(tmpfs_first_mount == 0) {
        for(int i = 0; i < 24; i ++) {
            tmpfs_filehandle[i].flags = HANDLE_NOT_USED;
        }
        tmpfs_first_mount = 1;
    }
    return 0;
}

int tmpfs_lookup(struct vnode* dir_node, struct vnode** target, const char * componenet_name) {

    struct ds_list_head *cur = dir_node->ch_list.next;
    struct vnode* v;
    struct tmpfs_vinfo *info;
    while(cur != &(dir_node->ch_list)) {
        v = container_of(cur, struct vnode, v_head);
        info = v->internal;
        if(strncmp(info->name, componenet_name, strlen(componenet_name)) == 0) {
            *target = v;
            return 0;
        }

        cur = ds_list_front(cur);
        if(cur == NULL) {
            return VFS_NOT_FOUND;
        }
    }
    return VFS_NOT_FOUND;
}

struct vnode* new_tmpfs_node(const char *name, struct vnode *dir_node) {
    struct vnode* ret = (struct vnode*)kmalloc(sizeof(struct vnode));


    if(dir_node != NULL) {
        ret->f_ops = dir_node->f_ops;
        ret->v_ops = dir_node->v_ops;
        ret->parent = dir_node;
    } else {
        ret->f_ops = &tmpfs_file_ops;
        ret->v_ops = &tmpfs_vnode_ops;
        ret->parent = NULL;
    }

    ret->mount = NULL;
    ds_list_head_init(&(ret->ch_list));
    ds_list_head_init(&(ret->v_head));
    ret->internal = (struct tmpfs_vinfo*)kmalloc(sizeof(struct tmpfs_vinfo));
    struct tmpfs_vinfo *ptr = ret->internal;
    int len = strlen(name);
    memcpy(&(ptr->name), name, len);
    ptr->namesize = len;
    ptr->content = NULL;
    ptr->filesize = 0;
    ptr->name[len] = '\0';
    return ret;
}


int tmpfs_create(struct vnode* dir_node, struct vnode** target, const char *component_name) {

    struct vnode* ret;
    int err = tmpfs_lookup(dir_node, &ret, component_name);
    if(err == VFS_NOT_FOUND) {
        ret = new_tmpfs_node(component_name, dir_node);
    }  else {
        struct tmpfs_vinfo *info = ret->internal;
        kfree(info->content);
        info->filesize = 0;
        info->content = NULL;
    }
    ds_list_addprev(&(dir_node->ch_list), &(ret->v_head));
    ret->parent = dir_node;
    *target = ret;
    return 0;
}

int tmpfs_mkdir(struct vnode* dir_node, struct vnode** target, const char *componenet_name) {
    struct vnode* ret = new_tmpfs_node(componenet_name, dir_node);
    ret->flag = VFS_DIR;
    ds_list_addprev(&(dir_node->ch_list), &(ret->v_head));
    ret->parent = dir_node;

    *target = ret;
    return 0;
}

void init_fs() {
    register_filesystem(&tmpfs);
    vfs_mount("/", "tmpfs");
    return;
}


int tmpfs_read(struct file* file, const void *buf, size_t len) {
    struct vnode *vn = file->vnode;
    struct tmpfs_vinfo *info = vn->internal;

    for(int i = 0; i < len; i ++, file->f_pos ++) {
        if(file->f_pos == info->filesize) {
            return i;
        }
        *(char *)(buf + i) = *(char *)(info->content + file->f_pos);
    }
    return len;
}

int tmpfs_open(struct vnode* file_node, struct file** target) {
    // uart_send_string("In tmpfs open\r\n");


    tmpfs_print_name(file_node);
    for(int i = 0; i < 24; i ++) {
        if(tmpfs_filehandle[i].flags == HANDLE_NOT_USED) {
            tmpfs_filehandle[i].vnode = file_node;
            tmpfs_filehandle[i].f_ops = &tmpfs_file_ops;
            tmpfs_filehandle[i].f_pos = 0;
            tmpfs_filehandle[i].flags = 0;
            *(target) = &(tmpfs_filehandle[i]);
            return i;
        }
    }
    return -1;
}

int tmpfs_close(struct file* file) {
    file->flags = HANDLE_NOT_USED;
    file->vnode = NULL;
    return 0;
}

long tmpfs_lseek64(struct file* file, long offset, int whence) {
    return 0;
}

int tmpfs_write(struct file *file, const void *buf, size_t len) {
    struct vnode* vn = file->vnode;
    struct tmpfs_vinfo *info = (struct tmpfs_vinfo*)file->vnode->internal;
    if(info->content == NULL) {
        info->content = (void*)cmalloc((1 << 12));
    }

    for(int j = 0; j < len; j ++, file->f_pos ++) {
        if(file->f_pos == TMPFS_MAX_CONTENT_SIZE) {
            return j;
        }
        *(char *)(info->content + file->f_pos) = *(char*)(buf + j);
        if(info->filesize <= file->f_pos) {
            info->filesize = file->f_pos + 1;
        }
    }
    return len;
}

void tmpfs_print_name(struct vnode* node) {
    struct tmpfs_vinfo *info = node->internal;

    uart_send_string(info->name);
    uart_send_string("\r\n");
}

int tmpfs_stat(struct file *f, struct fstat *stat) {
    struct tmpfs_vinfo *info = f->vnode->internal;
    (stat)->filesize = info->filesize;
    (stat)->namesize = info->namesize;
    return 0;
}