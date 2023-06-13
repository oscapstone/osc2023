#include "vfs.h"
#include "tmpfs.h"
#include "initramfs.h"
#include "fat32.h"
#include "string.h"
#include "mm.h"
#include "sched.h"
#include "mini_uart.h"

struct mount *rootfs;

void rootfs_init() {
    
    struct filesystem *tmpfs = (struct filesystem *)chunk_alloc(sizeof(struct filesystem));
    tmpfs->name = (char *)chunk_alloc(16);
    strcpy(tmpfs->name, "tmpfs");
    tmpfs->setup_mount = tmpfs_setup_mount;
    register_fs(tmpfs);
    
    rootfs = (struct mount *)chunk_alloc(sizeof(struct mount));    
    tmpfs->setup_mount(tmpfs, rootfs);

}

void initramfs_init() {
    
    vfs_mkdir("/initramfs");
    vfs_mount("/initramfs", "initramfs");
    parse_initramfs();

}

void fat32_init() {

    vfs_mkdir("/boot");
    vfs_mount("/boot", "fat32");
    parse_fat32sd();

}

int register_fs(struct filesystem *fs) {
    if (stringcmp(fs->name, "tmpfs") == 0) {
        return tmpfs_register();
    } else if (stringcmp(fs->name, "initramfs") == 0) {
        return initramfs_register();
    } else if (stringcmp(fs->name, "fat32") == 0) {
        return fat32_register();
    }
    return -1;
}

struct file* create_fd(struct vnode* target, int flags) {
    struct file* fd = (struct file*)chunk_alloc(sizeof(struct file));
    fd->f_ops = target->f_ops;
    fd->vnode = target;
    fd->f_pos = 0;
    fd->flags = flags;
    return fd;
}

int vfs_open(const char *pathname, int flags, struct file **target) {
    
    *target = 0;
    struct vnode *target_dir;
    char target_path[VFS_PATHMAX];
    traverse(pathname, &target_dir, target_path);
    
    struct vnode *target_file;
    if (target_dir->v_ops->lookup(target_dir, &target_file, target_path) == REGULAR_FILE) {
    
        *target = create_fd(target_file, flags);
        return (*target)->f_ops->open(target_file, target);

    } else if (flags & O_CREAT) {

        int res = target_dir->v_ops->create(target_dir, &target_file, target_path);
        if (res < 0) return FAIL;
        *target = create_fd(target_file, flags);
        return (*target)->f_ops->open(target_file, target);

    } else return FAIL;

}

int vfs_close(struct file *file) {
    int code = file->f_ops->close(file);
    if (code == SUCCESS)
        chunk_free(file);
    return code;
}

int vfs_write(struct file *file, const void *buf, unsigned len) {
    return file->f_ops->write(file, buf, len);
}

int vfs_read(struct file *file, void *buf, unsigned len) {
    return file->f_ops->read(file, buf, len);
}

int vfs_mkdir(const char *pathname) {
    struct vnode *target_dir;
    char child_name[VFS_PATHMAX];
    traverse(pathname, &target_dir, child_name);
    struct vnode *child_dir;
    int res = target_dir->v_ops->mkdir(target_dir, &child_dir, child_name);
    if (res < 0) return res;
    return SUCCESS;
}

int vfs_mount(const char *target, const char *filesystem) {
    
    struct vnode *mount_dir;
    char path_remain[VFS_PATHMAX];

    traverse(target, &mount_dir, path_remain);
    
    struct mount *mt = (struct mount *)chunk_alloc(sizeof(struct mount));
    if (stringcmp(filesystem, "tmpfs") == 0) {
        
        struct filesystem *tmpfs = (struct filesystem *)chunk_alloc(sizeof(struct filesystem));
        tmpfs->name = (char *)chunk_alloc(16);
        strcpy(tmpfs->name, "tmpfs");
        tmpfs->setup_mount = tmpfs_setup_mount;
        register_fs(tmpfs);
        tmpfs->setup_mount(tmpfs, mt);
        mount_dir->mount = mt;
        mt->root->parent = mount_dir->parent;

    } else if (stringcmp(filesystem, "initramfs") == 0) {

        struct filesystem *initramfs = (struct filesystem *)chunk_alloc(sizeof(struct filesystem));
        initramfs->name = (char *)chunk_alloc(16);
        strcpy(initramfs->name, "initramfs");
        initramfs->setup_mount = initramfs_setup_mount;
        register_fs(initramfs);
        initramfs->setup_mount(initramfs, mt);
        mount_dir->mount = mt;
        mt->root->parent = mount_dir->parent;

    } else if (stringcmp(filesystem, "fat32") == 0) {

        struct filesystem *fat32 = (struct filesystem *)chunk_alloc(sizeof(struct filesystem));
        fat32->name = (char *)chunk_alloc(16);
        strcpy(fat32->name, "fat32");
        fat32->setup_mount = fat32_setup_mount;
        register_fs(fat32);
        fat32->setup_mount(fat32, mt);
        mount_dir->mount = mt;
        mt->root->parent = mount_dir->parent;

    }

    return SUCCESS;
}

int vfs_lookup(const char *pathname, struct vnode **target) {
    return SUCCESS;
}

int vfs_chdir(const char *pathname) {
    struct vnode *target_dir;
    char path_remain[VFS_PATHMAX];
    traverse(pathname, &target_dir, path_remain);
    if (stringcmp(path_remain, "") != 0) {
        return FAIL;
    } else {
        
        current->cwd = target_dir;
        return SUCCESS;
    }
}

void traverse(const char* pathname, struct vnode **target_node, char *target_path) {
    if (pathname[0] == '/') {
        printf("[debug] traverse absolute path: %s\n", pathname);
        struct vnode *rootnode = rootfs->root;
        r_traverse(rootnode, pathname + 1, target_node, target_path);
    } else {
        printf("[debug] traverse relative path: %s\n", pathname);
        struct vnode *rootnode = current->cwd;
        r_traverse(rootnode, pathname, target_node, target_path);
    }
}

void r_traverse(struct vnode *node, const char *path, struct vnode **target_node, char *target_path) {
    
    int i = 0;
    while (path[i]) {
        if (path[i] == '/') break;
        target_path[i] = path[i];
        i++;
    }
    target_path[i++] = '\0';
    *target_node = node;

    if (stringcmp(target_path, "") == 0) {
        return;
    }
    else if (stringcmp(target_path, ".") == 0) {
        r_traverse(node, path + i, target_node, target_path);
        return;
    }
    else if (stringcmp(target_path, "..") == 0) {
        if (node->parent == NULL) return;
        r_traverse(node->parent, path + i, target_node, target_path);
        return;
    }

    int res = node->v_ops->lookup(node, target_node, target_path); printf("[debug] lookup ret: %d\n", res);
    if ((*target_node)->mount != NULL) {
        printf("[debug] mountpoint found during lookup: vnode 0x%x\n", (*target_node)->mount->root);
        r_traverse((*target_node)->mount->root, path+i, target_node, target_path);
    }
    else if (res == DIRECTORY) 
        r_traverse(*target_node, path+i, target_node, target_path);
    else if (res == REGULAR_FILE)
        *target_node = node;
    else if (res == FAIL && node->v_ops->load_vnode != NULL) {
        int ret = node->v_ops->load_vnode(node, target_path);
        if (ret == SUCCESS) {
            r_traverse(node, path, target_node, target_path);
        }
    }
    
}
