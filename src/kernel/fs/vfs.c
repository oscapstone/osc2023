#include "fs/vfs.h"
#include "mem/mem.h"
#include "utils.h"
#include "ds/list.h"
#include "interrupt.h"
#include "peripherals/mini_uart.h"
#include "process.h"


struct mount* rootfs;
struct ds_list_head fs_list;
void vfs_init(){
    rootfs = NULL;
    ds_list_head_init(&fs_list);
    return;
}

struct vnode* get_parent_vnode(struct vnode* node) {
    return node->parent;
}

struct filesystem *find_fs_by_name(const char *name) {
    // struct ds_list_head *head = ds_list_front(&fs_list);
    struct ds_list_head *head = fs_list.next;
    // if(head == NULL) {
    //     return NULL;
    // }
    while(head != &fs_list) {
        struct filesystem *fs = container_of(head, struct filesystem, fs_head);
        // uart_send_string(fs->name);
        if(strncmp(name, fs->name, strlen(fs->name)) == 0) {
            return fs;
        }
        head = ds_list_front(head);
        if(head == NULL) {
            return NULL;
        }
    }
    return NULL;
}

int register_filesystem(struct filesystem* fs) {
    
    struct filesystem *same = find_fs_by_name(fs->name);
    if(same == NULL) {
        if(fs->init_fs != NULL) {
            fs->init_fs(fs);
        }
        ds_list_head_init(&(fs->fs_head));
        ds_list_addprev(&fs_list, &(fs->fs_head));
        return 0;
    } else {
        return VFS_NOT_FOUND;
    }
}


// this is very bad implementation
// can only fit comp[20][20] array
int split_path(const char* pathname, char comp[][20]) {
    
    int len = strlen(pathname);
    int beg = 0;
    int end = 0;
    int idx = 0;

    for(int i = 0; i < len; i ++) {
        if(pathname[i] == '/') {
            end = i;
            if(end - beg > 0) {
                memcpy(comp[idx], pathname + beg, end - beg);
                comp[idx][end - beg] = '\0';
                idx ++;
            }
            beg = i + 1;
        }
    }
    char *ptr = comp[idx];
    if(len != beg) {
        memcpy(ptr, pathname + beg, len - beg);
        *(char *)(ptr + len - beg) = '\0';
        idx += 1;
    }
    return idx;
}


int vfs_lookup(const char* pathname, struct vnode** target) {



    struct vnode* vnode_itr;

    if(pathname[0] == '/') {
        // uart_send_string("From root\r\n");
        vnode_itr = rootfs->root;
    } else {
        struct Process_t *proc = process_get_current();
        vnode_itr = proc->cur_vnode;
    }

    char comp[20][20];

    int num = split_path(pathname, &comp);
    *target = NULL;

    for(int i = 0; i < num; i ++) {
        // uart_send_string(comp[i]);
        // int x = strncmp(comp[i], "..", 2);
        // uart_send_dec(x);
        if(strncmp(comp[i], "..", 2) == 0) {
            vnode_itr = vnode_itr->parent;
            continue;
        } else if(strncmp(comp[i], ".", 1) == 0) {
            continue;
        } 
        struct vnode* next_vnode;
        // uart_send_u64(vnode_itr);
        // uart_send_string("before this\r\n");
        struct vnode* ret = vnode_itr->v_ops->lookup(vnode_itr, &next_vnode, comp[i]);
        // uart_send_string("After this\r\n");
        if(ret != 0) {
            // if this is last one
            if(i == num - 1) {
                *target = vnode_itr;
                return VFS_CAN_CREATE;
            }
            return ret;
        }
        if(next_vnode->mount != NULL) {
            // uart_send_string("Jump to mounted vnode \r\n");
            // uart_send_u64(next_vnode->mount->root);
            // uart_send_string("\r\n");
            vnode_itr = next_vnode->mount->root;
        } else {
            vnode_itr = next_vnode;
        }
    }

    *target = vnode_itr;
    return 0;
}

// api spec:
// different filesystem should provide its own root vnode.
int vfs_mount(const char *target, const char *filesystem) {
    // uart_send_string(target);
    // uart_send_string("\r\n");
    // uart_send_string(filesystem);
    // uart_send_string("\r\n");
    struct filesystem* fs = find_fs_by_name(filesystem);
    if(fs == NULL) {
        // uart_send_string(filesystem);
        // uart_send_string(" not found\r\n");
        return VFS_NOT_FOUND;
    }
    if(rootfs == NULL) {
        rootfs = (struct mount *)kmalloc(sizeof(struct mount));
        rootfs->mountpoint = NULL;
        int err = fs->setup_mount(fs, rootfs);
        rootfs->root->parent = rootfs->root;
        return err;
    } else {
        struct vnode *v;
        int err = vfs_lookup(target, &v);
        if(v->flag != VFS_DIR) {
            return VFS_NOT_DIR;
        }

        if(v->mount != NULL) {
            return VFS_MOUNTED;
        }
        if(err) {
            // uart_send_string("Error when mounting\r\n");
            return err;
        }
        v->mount = (struct mount *)kmalloc(sizeof(struct mount));
        err = fs->setup_mount(fs, v->mount);
        if(err == 0) {
            if(v->mount == fs->root->mount) {
                v->mount->root->parent = v->parent;
                v->mount->mountpoint = v;
            }
        } 
        return err;
    }
}

int vfs_open(const char* pathname, int flags, struct file** target) {
    // 1. Lookup pathname
    // 2. Create a new file handle for this vnode if found.
    // 3. Create a new file if O_CREAT is specified in flags and vnode not found
    // lookup error code shows if file exist or not or other error occurs
    // 4. Return error code if fails
    // uart_send_string("OPening flags\r\n");
    // uart_send_dec(flags);
    // uart_send_string("\r\n");
    struct vnode *v;
    // uart_send_string("hello\r\n");
    int err = vfs_lookup(pathname, &v);
    char comp[20][20];
    // uart_send_string("hello\r\n");
    int num = split_path(pathname, comp);
    if(err) {
        if(err == VFS_CAN_CREATE && (flags & O_CREAT)) {
            struct vnode *tmpv = v;
            err = tmpv->v_ops->create(tmpv, &v, comp[num - 1]);
            if(err) {
                return err;
            }
        } else {
            return err;
        } 
    } else {
        if(flags & O_CREAT) {
            struct vnode *tmpv = v;
            err = tmpv->parent->v_ops->create(tmpv->parent, &v, comp[num - 1]);
            if(err) {
                return err;
            }
        }
    }
    int fd = v->f_ops->open(v, target);
    if(fd > 0) {
        (*target)->flags = flags;
        return 0;
    }
    return fd;
}

int vfs_close(struct file* file) {
    // 1. release the file handle
    // 2. Return error code if fails
    return file->f_ops->close(file);
}

int vfs_write(struct file* file, const void* buf, size_t len) {
    return file->f_ops->write(file, buf, len);
}

int vfs_read(struct file* file, void* buf, size_t len) {
    return file->f_ops->read(file, buf, len);
}

void vfs_apply_dir(struct vnode* dir_node, apply_dir_func func) {
    struct ds_list_head *head = ds_list_front(&(dir_node->ch_list));
    if(head == NULL) {
        return;
    }

    while(head != &(dir_node->ch_list)) {
        struct vnode* node = container_of(head, struct vnode, v_head);
        func(node);
        head = ds_list_front(head);
    }
}

int vfs_create(const char *pathname, int flags, struct vnode** target) {
    struct vnode *ret;
    int err = vfs_lookup(pathname, &ret);


    if(err == 0) {
        *(target) = ret;
        return VFS_EXIST;
    } else if(err == VFS_CAN_CREATE) {
        char comp[20][20];
        int num = split_path(pathname, comp);
        struct vnode *tmpret = ret;
        err = tmpret->v_ops->create(tmpret, &ret, comp[num - 1]);
        *(target) = ret;
        return err;
    } else {
        return err;
    }
}

int vfs_mkdir(const char *pathname) {

    struct vnode *v;
    
    int err = vfs_lookup(pathname, &v);
    if(err == 0) {
        return VFS_EXIST;
    } else if(err == VFS_CAN_CREATE) {
        char comp[20][20];
        int num = split_path(pathname, comp);
        err = v->v_ops->mkdir(v, &v, comp[num - 1]);
        return err;
    } else {
        return err;
    }
}

int vfs_stat(struct file *f, struct fstat *stat) {
    return f->f_ops->stat(f, stat);
}

int vops_not_support() {
    return VFS_NOT_SUPPORT;
}
int fops_not_support() {
    return VFS_NOT_SUPPORT;
}

int vfs_lseek64(struct file *f, long offset, int whence) {
    return f->f_ops->lseek64(f, offset, whence);
}