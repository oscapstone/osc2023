#include "vfs.h"
#include "tmpfs.h"
#include "devfs.h"
#include "fat32.h"
#include "stdlib.h"

filesystem_t *fs_list[50];
unsigned int fs_count = 0;
struct mount *rootfs;
char cwdpath[256];
file_t *kfd[16];
int fd_count = 0;

int register_filesystem(struct filesystem *fs)
{
    // register the file system to the kernel.
    for (int i = 0; i < fs_count; i++)
    {
        if (!strcmp(fs->name, fs_list[i]->name))
            return -1;
    }

    fs_list[fs_count] = fs;
    fs_count++;

    return 0;
    // you can also initialize memory pool of the file system here.
}

int vfs_mount(char *target, char *filesystem)
{
    if (!strcmp(target, "/"))
    {
        rootfs = my_malloc(sizeof(mount_t));
        rootfs->fs = my_malloc(sizeof(filesystem_t));
        rootfs->fs->name = my_malloc(strlen(filesystem) + 1);
        strcpy(rootfs->fs->name, filesystem);
        rootfs->fs->setup_mount = tmpfs_mount;
        register_filesystem(rootfs->fs);
        return rootfs->fs->setup_mount(rootfs->fs, rootfs);
    }
    else if (!strcmp(target, "/initramfs"))
    {
        vnode_t *dir_node = NULL;

        vfs_mkdir(target);
        vfs_lookup(target, &dir_node);

        dir_node->mount = my_malloc(sizeof(mount_t));
        dir_node->mount->root = dir_node;
        dir_node->mount->fs = my_malloc(sizeof(filesystem_t));
        dir_node->mount->fs->name = my_malloc(strlen(filesystem) + 1);
        strcpy(dir_node->mount->fs->name, filesystem);
        dir_node->mount->fs->setup_mount = initramfs_mount;
        register_filesystem(dir_node->mount->fs);
        return dir_node->mount->fs->setup_mount(dir_node->mount->fs, dir_node->mount);
    }
    else if (!strcmp(target, "/dev"))
    {
        vnode_t *dir_node = NULL;

        vfs_mkdir(target);
        vfs_lookup(target, &dir_node);

        dir_node->mount = my_malloc(sizeof(mount_t));
        dir_node->mount->root = dir_node;
        dir_node->mount->fs = my_malloc(sizeof(filesystem_t));
        dir_node->mount->fs->name = my_malloc(strlen(filesystem) + 1);
        strcpy(dir_node->mount->fs->name, filesystem);
        dir_node->mount->fs->setup_mount = devfs_setup_mount;
        register_filesystem(dir_node->mount->fs);
        return dir_node->mount->fs->setup_mount(dir_node->mount->fs, dir_node->mount);
    }
    else if (!strcmp(target, "/boot"))
    {
        vnode_t *dir_node = NULL;

        vfs_mkdir(target);
        vfs_lookup(target, &dir_node);

        dir_node->mount = my_malloc(sizeof(mount_t));
        dir_node->mount->root = dir_node;
        dir_node->mount->fs = my_malloc(sizeof(filesystem_t));
        dir_node->mount->fs->name = my_malloc(strlen(filesystem) + 1);
        strcpy(dir_node->mount->fs->name, filesystem);
        dir_node->mount->fs->setup_mount = fat32fs_mount;
        register_filesystem(dir_node->mount->fs);
        return dir_node->mount->fs->setup_mount(dir_node->mount->fs, dir_node->mount);
    }
    else
    {
        vnode_t *dir_node = NULL;

        switch (vfs_lookup(target, &dir_node))
        {
        case EXISTED:
            if (dir_node->internal->type != DIR)
            {
                printf("%s is not a directory.\n", target);
                return -1;
            }
            break;
        case NOTDIR:
            return -1;
        case NOTFOUND:
            printf("%s is not existed.\n", target);
            return -1;
        }

        dir_node->mount = my_malloc(sizeof(mount_t));
        dir_node->mount->root = dir_node;
        dir_node->mount->fs = my_malloc(sizeof(filesystem_t));
        dir_node->mount->fs->name = my_malloc(strlen(filesystem) + 1);
        strcpy(dir_node->mount->fs->name, filesystem);
        // if mounting to a existed directory, all original content will not be used temporarily. When unmounted, will recover the contents.
        // TODO : keep original "vnode->internal", use new one replace it. And change v_ops & f_ops to new filesystem's
        if (register_filesystem(dir_node->mount->fs) == -1)
        {
            char dir_name[COMPONENT_NAME_MAX];
            strcpy(dir_name, dir_node->internal->name);

            dir_node->internal = my_malloc(sizeof(node_info_t));
            dir_node->internal->name = my_malloc(strlen(dir_name) + 1);
            strcpy(dir_node->internal->name, dir_name);

            dir_node->internal->type = DIR;
            dir_node->internal->size = 0;
            dir_node->internal->entry = my_malloc(sizeof(vnode_t *) * MAX_NUM_OF_ENTRY);

            return 0;
        }
        return dir_node->mount->fs->setup_mount(dir_node->mount->fs, dir_node->mount);
    }

    return -1;
}

int vfs_lookup(char *pathname, struct vnode **target)
{
    char abs_pathname[PATHNAME_MAX];
    handle_path(pathname, abs_pathname);

    if (!strcmp("/", abs_pathname))
    {
        *target = rootfs->root;
        return 0;
    }

    vnode_t *vnode = rootfs->root;
    int level = 0;
    char next_component[COMPONENT_NAME_MAX];
    int total_level = strnchr(abs_pathname, '/');

    while (level < total_level)
    {
        get_next_component(abs_pathname, next_component, level);

        vnode_t *next_node = NULL;
        int ret = vnode->v_ops->lookup(vnode, &next_node, next_component);

        if (ret != EXISTED)
            return ret;

        vnode = next_node;
        level++;
    }

    *target = vnode;

    return EXISTED;
}

int vfs_create(char *pathname)
{
    vnode_t *dir_node = NULL;
    char abs_pathname[PATHNAME_MAX];

    handle_path(pathname, abs_pathname);

    switch (vfs_lookup(abs_pathname, &dir_node))
    {
    case EXISTED:
        printf("%s is existed.\n", abs_pathname);
        return -1;
    case NOTDIR:
        return -1;
    }

    char target_component[COMPONENT_NAME_MAX];
    char dir_pathname[PATHNAME_MAX];

    basename(abs_pathname, target_component);
    dirname(abs_pathname, dir_pathname);
    if (vfs_lookup(dir_pathname, &dir_node) == NOTFOUND)
    {
        printf("%s not found.\n", dir_pathname);
        return -1;
    }

    vnode_t *target = NULL;

    int ret = dir_node->v_ops->create(dir_node, &target, target_component);

    if (ret != 0)
        return ret;

    return 0;
}

int vfs_mkdir(char *pathname)
{
    vnode_t *dir_node = NULL;
    char abs_pathname[PATHNAME_MAX];

    handle_path(pathname, abs_pathname);

    switch (vfs_lookup(abs_pathname, &dir_node))
    {
    case EXISTED:
        printf("%s is existed.\n", abs_pathname);
        return -1;
    case NOTDIR:
        return -1;
    }

    char target_component[COMPONENT_NAME_MAX];
    char dir_pathname[PATHNAME_MAX];

    basename(abs_pathname, target_component);
    dirname(abs_pathname, dir_pathname);
    if (vfs_lookup(dir_pathname, &dir_node) == NOTFOUND)
    {
        printf("%s not found.\n", dir_pathname);
        return -1;
    }

    vnode_t *target = NULL;
    int ret = dir_node->v_ops->mkdir(dir_node, &target, target_component);

    if (ret != 0)
        return ret;

    return 0;
}

int vfs_open(char *pathname, int flags, struct file **target)
{
    // 1. Lookup pathname
    // 2. Create a new file handle for this vnode if found.
    // 3. Create a new file if O_CREAT is specified in flags and vnode not found
    // lookup error code shows if file exist or not or other error occurs
    // 4. Return error code if fails
    vnode_t *target_node = NULL;
    char abs_pathname[PATHNAME_MAX];

    handle_path(pathname, abs_pathname);

    int ret = vfs_lookup(abs_pathname, &target_node);

    if (ret == EXISTED && target_node->internal->type == FILE)
    {
        return target_node->f_ops->open(target_node, target);
    }
    else if (ret == EXISTED && target_node->internal->type == DIR)
    {
        printf("%s is not a file.\n", abs_pathname);
        return -1;
    }
    else if (ret == NOTFOUND && (flags & O_CREAT))
    {
        if (vfs_create(abs_pathname) != 0)
            return -1;
        vfs_lookup(abs_pathname, &target_node);
        return target_node->f_ops->open(target_node, target);
    }
    else if (ret == NOTFOUND)
    {
        printf("[DEBUG/vfs_open] %s is not existed.\n", abs_pathname);
        return -1;
    }
    else
        return -1;
}

int vfs_close(struct file *file)
{
    // 1. release the file handle
    // 2. Return error code if fails
    if (file == NULL)
    {
        printf("file is not existed.\n");
        return -1;
    }
    return file->f_ops->close(file);
}

int vfs_write(struct file *file, void *buf, size_t len)
{
    // 1. write len byte from buf to the opened file.
    // 2. return written size or error code if an error occurs.
    if (file == NULL)
    {
        printf("file is not existed.\n");
        return -1;
    }
    return file->f_ops->write(file, buf, len);
}

int vfs_read(struct file *file, void *buf, size_t len)
{
    // 1. read min(len, readable size) byte to buf from the opened file.
    // 2. block if nothing to read for FIFO type
    // 3. return read size or error code if an error occurs.
    if (file == NULL)
    {
        printf("file is not existed.\n");
        return -1;
    }
    return file->f_ops->read(file, buf, len);
}

void get_next_component(char *pathname, char *target, int level)
{
    int level_count = -1;
    int target_begin = 0;
    while (level_count != level)
    {
        if (pathname[target_begin] == '/')
            level_count++;
        target_begin++;
    }

    int i = 0;
    while (pathname[target_begin + i] != '/' && pathname[target_begin + i] != '\0')
    {
        target[i] = pathname[target_begin + i];
        i++;
    }

    target[i] = '\0';
}

void basename(char *src, char *des)
{
    int level = 0;
    int total_level = strnchr(src, '/');

    if (!strcmp(src, "/"))
    {
        strcpy(des, "/");
        return;
    }

    while (level < total_level)
    {
        get_next_component(src, des, level);
        level++;
    }
}

void dirname(char *src, char *des)
{
    int end = -1;

    for (int i = 0; i < strlen(src); i++)
        if (src[i] == '/')
            end = i;

    if (end == 0)
        strcpy(des, "/");
    else
        strncpy(src, des, end);
}

void handle_path(char *rela, char *abso)
{
    if (rela[0] == '/')
    {
        strcpy(abso, rela);
        if (abso[strlen(abso) - 1] == '/' && strlen(abso) != 1)
            abso[strlen(abso) - 1] = '\0';
        return;
    }

    strcpy(abso, cwdpath);
    char next_component[COMPONENT_NAME_MAX];

    int i = 0, j = 0;
    while (rela[i] != '\0')
    {
        if (rela[i] != '/')
        {
            next_component[j] = rela[i];
            j++;
        }
        else if (rela[i] == '/' && i == strlen(rela) - 1)
            break;
        else
        {
            next_component[j] = '\0';
            j = 0;

            if (!strcmp(next_component, "."))
                ;
            else if (!strcmp(next_component, ".."))
                dirname(abso, abso);
            else
            {
                if (abso[strlen(abso) - 1] != '/')
                    strcat(abso, "/");
                strcat(abso, next_component);
            }
        }

        i++;
    }

    next_component[j] = '\0';

    if (!strcmp(next_component, "."))
        return;
    else if (!strcmp(next_component, ".."))
        dirname(abso, abso);
    else
    {
        if (abso[strlen(abso) - 1] != '/')
            strcat(abso, "/");
        strcat(abso, next_component);
    }
}

void vfs_ls(char *pathname, int flag)
{
    vnode_t *target_vnode;
    int ret;

    if (flag)
        ret = vfs_lookup(pathname, &target_vnode);
    else
        ret = vfs_lookup(cwdpath, &target_vnode);

    switch (ret)
    {
    case NOTFOUND:
        printf("%s is not existed.\n", pathname);
        return;
    case NOTDIR:
        return;
    }

    if (target_vnode->internal->type != DIR)
    {
        printf("%s is not a directory.\n", pathname);
        return;
    }

    for (int i = 0; i < target_vnode->internal->size; i++)
    {
        printf("%s", target_vnode->internal->entry[i]->internal->name);
        if (target_vnode->internal->entry[i]->internal->type == DIR)
            printf("/");
        printf("    ");
    }
    printf("\n");
}

int vfs_cd(char *target_dir)
{
    vnode_t *dir_node = NULL;

    switch (vfs_lookup(target_dir, &dir_node))
    {
    case EXISTED:
        if (dir_node->internal->type != DIR)
        {
            printf("%s is not a directory.\n", target_dir);
            return -1;
        }
        break;
    case NOTDIR:
        return -1;
    case NOTFOUND:
        printf("%s is not existed.\n", target_dir);
        return -1;
    }

    strcpy(cwdpath, target_dir);

    return 0;
}

long vfs_lseek(struct file *file, long offset, int whence)
{
    if (file == NULL)
    {
        printf("file is not existed.\n");
        return -1;
    }
    return file->f_ops->lseek64(file, offset, whence);
}

long vfs_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int error = 0;
    printf("ioctl() unimplement, return 0\n");
    return error;
}