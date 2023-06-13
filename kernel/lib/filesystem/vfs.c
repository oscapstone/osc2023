#include "filesystem/vfs.h"
#include "filesystem/tmpfs.h"
#include "malloc.h"
#include "string.h"
#include "filesystem/initramfs.h"
#include "filesystem/dev_uart.h"
#include "filesystem/dev_framebuffer.h"

struct mount *rootfs;
struct filesystem reg_fs[MAX_FS_REG];
struct file_operations reg_dev[MAX_DEV_REG];

int register_filesystem(struct filesystem *fs)
{
    for (int i = 0; i < MAX_FS_REG; i++)
    {
        if (!reg_fs[i].name)
        {
            reg_fs[i].name = fs->name;
            reg_fs[i].setup_mount = fs->setup_mount;
            return i;
        }
    }
    return -1;
}

int register_dev(struct file_operations *fo)
{
    // register device's file operations
    for (int i = 0; i < MAX_FS_REG; i++)
    {
        if (!reg_dev[i].open)
        {
            reg_dev[i] = *fo;
            return i;
        }
    }
    return -1;
}

struct filesystem *find_filesystem(const char *fs_name)
{
    for (int i = 0; i < MAX_FS_REG; i++)
    {
        if (strcmp(reg_fs[i].name, fs_name) == 0)
            return &reg_fs[i];
    }
    return 0;
}

int vfs_open(const char *pathname, int flags, struct file **target)
{
    // 1. Lookup pathname
    // 3. Create a new file if O_CREAT is specified in flags and vnode not found
    struct vnode *node;
    if (vfs_lookup(pathname, &node) != 0 && (flags & O_CREAT))
    {
        int last_slash_idx = 0;
        // get last slash
        for (int i = 0; i < strlen(pathname); i++)
            if (pathname[i] == '/')
                last_slash_idx = i;

        // dir name
        char dirname[MAX_PATH_NAME + 1];
        strcpy(dirname, pathname);
        dirname[last_slash_idx] = 0;
        if (vfs_lookup(dirname, &node) != 0)
        {
            uart_printf("cannot ocreate no dir name\n");
            return -1;
        }
        // create file by calling create operation
        node->v_ops->create(node, &node, pathname + last_slash_idx + 1);
        // create file handler
        *target = malloc(sizeof(struct file));
        node->f_ops->open(node, target);
        (*target)->flags = flags;
        return 0;
    }
    else // 2. Create a new file handle for this vnode if found.
    {   
        // file exists
        *target = malloc(sizeof(struct file));
        node->f_ops->open(node, target);
        (*target)->flags = flags;
        return 0;
    }

    // lookup error code shows if file exist or not or other error occurs
    // 4. Return error code if fails
    return -1;
}

int vfs_close(struct file *file)
{
    // 1. release the file handle
    // 2. Return error code if fails
    file->f_ops->close(file);
    return 0;
}

int vfs_write(struct file *file, const void *buf, size_t len)
{
    // 1. write len byte from buf to the opened file.
    // 2. return written size or error code if an error occurs.
    return file->f_ops->write(file, buf, len);
}

int vfs_read(struct file *file, void *buf, size_t len)
{
    // 1. read min(len, readable size) byte to buf from the opened file.
    // 2. block if nothing to read for FIFO type
    // 2. return read size or error code if an error occurs.
    return file->f_ops->read(file, buf, len);
}

int vfs_mkdir(const char *pathname)
{
    // e.g : /a/b/c
    // dirname : /a/b
    // newdirname : c
    char dirname[MAX_PATH_NAME] = {};
    char newdirname[MAX_PATH_NAME] = {};

    // split pathname
    int last_slash_idx = 0;
    for (int i = 0; i < strlen(pathname); i++)
        if (pathname[i] == '/')
            last_slash_idx = i;

    memcpy(dirname, pathname, last_slash_idx);
    strcpy(newdirname, pathname + last_slash_idx + 1);

    struct vnode *node;
    // find corresponding vnode and call mkdir
    if (vfs_lookup(dirname, &node) == 0)
    {
        node->v_ops->mkdir(node, &node, newdirname);
        return 0;
    }

    uart_printf("vfs_mkdir cannot find pathname");
    return -1;
}

int vfs_mount(const char *target, const char *filesystem)
{
    struct vnode *dirnode;
    // find register filesystem

    struct filesystem *fs = find_filesystem(filesystem);
    if (!fs)
    {
        uart_printf("vfs_mount cannot find filesystem\n");
        return -1;
    }
    // find mount point
    if (vfs_lookup(target, &dirnode) == -1)
    {
        uart_printf("vfs_mount cannot find dir\n");
        return -1;
    }
    else
    {
        /* notice :
            setup_mount will new a vnode (target duplication) with different file_operation
            struct mount of taget's vnode will point to it
            when lookup switch filesystem will jump to new vnode
        */
        dirnode->mount = malloc(sizeof(struct mount));
        // filesystem setup mount
        fs->setup_mount(fs, dirnode->mount);
    }
    return 0;
}

int vfs_lookup(const char *pathname, struct vnode **target)
{
    if (strlen(pathname) == 0)
    {
        *target = rootfs->root;
        return 0;
    }

    struct vnode *dirnode = rootfs->root;
    char component_name[FILE_NAME_MAX + 1] = {};
    int c_idx = 0;
    // iterate through directory
    // e.g: lookup "a/b/c"
    /*
        use rootfs->lookup find vnode of a
        use a->lookup find vnode of b
        use b->lookup find vnode of c
    */
    for (int i = 1; i < strlen(pathname); i++)
    {
        if (pathname[i] == '/')
        {
            component_name[c_idx++] = 0;
            // use parent vnode lookup to file vnod
            if (dirnode->v_ops->lookup(dirnode, &dirnode, component_name) != 0)
                return -1;

            // redirect to new mounted filesystem
            while (dirnode->mount)
                dirnode = dirnode->mount->root;

            c_idx = 0;
        }
        else
        {
            component_name[c_idx++] = pathname[i];
        }
    }

    component_name[c_idx++] = 0;
    if (dirnode->v_ops->lookup(dirnode, &dirnode, component_name) != 0)
        return -1;
    // redirect to new mounted filesystem
    while (dirnode->mount)
        dirnode = dirnode->mount->root;

    // pass vnode
    *target = dirnode;

    return 0;
}

int vfs_mknod(char *pathname, int id)
{
    struct file *f = malloc(sizeof(struct file));
    // create file
    vfs_open(pathname, O_CREAT, &f);
    // assign registered file operation to vnode file operation
    // according register device id
    f->vnode->f_ops = &reg_dev[id];
    vfs_close(f);
    return 0;
}

long vfs_lseek64(struct file *file, long offset, int whence)
{   
    // assign file position to get offset
    if (whence == SEEK_SET)
    {
        file->f_pos = offset;
        return file->f_pos;
    }
    return -1;
}

void init_rootfs()
{
    int idx = register_tmpfs();
    rootfs = malloc(sizeof(struct mount));
    reg_fs[idx].setup_mount(&reg_fs[idx], rootfs);

    // for initramfs
    vfs_mkdir("/initramfs");
    // assign filename and setup mount callback
    register_initramfs();
    vfs_mount("/initramfs", "initramfs");

    // for dev
    vfs_mkdir("/dev");
    // mknod according devive id
    // register dev uart
    int uart_id = init_dev_uart();
    vfs_mknod("/dev/uart", uart_id);
    // setting mailbox
    int framebuffer_id = init_dev_framebuffer();
    vfs_mknod("/dev/framebuffer", framebuffer_id);
}

// syscall filepath is relative path
// we need to use relative path and cwd to get absolute path
char *path_to_absolute(char *path, char *cwd)
{
    // relative path
    // append to cwd
    if (path[0] != '/')
    {
        char tmp[MAX_PATH_NAME];
        strcpy(tmp, cwd);
        if (strcmp(cwd, "/") != 0)
            strcat(tmp, "/");
        strcat(tmp, path);
        strcpy(path, tmp);
    }

    char absolute_path[MAX_PATH_NAME + 1] = {};
    int idx = 0;
    // trim ".." and "."
    //
    for (int i = 0; i < strlen(path); i++)
    {
        // meet /..
        // return upper level
        if (path[i] == '/' && path[i + 1] == '.' && path[i + 2] == '.')
        {
            for (int j = idx; j >= 0; j--)
            {
                if (absolute_path[j] == '/')
                {
                    absolute_path[j] = 0;
                    idx = j;
                }
            }
            i += 2;
            continue;
        }

        // ignore /.
        if (path[i] == '/' && path[i + 1] == '.')
        {
            i++;
            continue;
        }

        absolute_path[idx++] = path[i];
    }

    absolute_path[idx] = 0;

    
    return strcpy(path, absolute_path);
}

int op_deny()
{
    return -1;
}