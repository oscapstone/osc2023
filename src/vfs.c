#include "vfs.h"
#include "tmpfs.h"
#include "utils.h"
#include "mm.h"
#include "thread.h"
#include "uart.h"
#include "list.h"
#include "initramfs.h"
#include "dev.h"
#include "exception.h"

struct mount* rootfs;

struct mount *new_mnt(
  struct vnode *root,
  struct filesystem *fs
)
{
    struct mount *ret = (struct vnode *)kmalloc(sizeof(struct vnode));
    if (!ret) return NULL;
    ret->root = root;
    ret->fs = fs;
    return ret;
}

struct vnode *new_vnode(
    struct mount *mount, 
    struct vnode_operations* v_ops, 
    struct file_operations* f_ops, 
    struct vnode *parent, 
    enum FileType file_type, 
    void *internal
)
{
    struct vnode *ret = (struct vnode *)kmalloc(sizeof(struct vnode));
    if (!ret) return NULL;
    ret->mount = mount;
    ret->v_ops = v_ops;
    ret->f_ops = f_ops;
    ret->parent = parent;
    ret->file_type = file_type;
    ret->internal = internal;
    return ret;
}

extern struct dentry *new_dentry(
  char *d_name,
  enum FileType file_type,
  void *content,
  const size_t *file_size
)
{
    struct dentry *ret = (struct dentry *)kmalloc(sizeof(struct dentry));
    if (!ret) return NULL;
    strncpy(ret->d_name, d_name, 31);
    *(enum FileType *)&(ret->file_type) = file_type;
    ret->content = content;
    ret->file_size = file_size;
    INIT_LIST_HEAD(&ret->file_link_node);
    INIT_LIST_HEAD(&ret->node);
    return ret;
}

LIST_HEAD(fs_list);
static struct filesystem *fs_lookup(const char *name)
{
    struct filesystem *entry, *safe;
    list_for_each_entry_safe(entry, safe, &fs_list, node) {
        if (strcmp(name, entry->name) == 0) {
            return entry;
        }
    }
    return NULL;
}

struct filesystem *new_fs(const char *name, int (*setup_mount)(struct filesystem* fs, struct mount* mount))
{
    struct filesystem *p = (struct filesystem *)kmalloc(sizeof(struct filesystem));
    if (p == NULL) return NULL;
    p->name = name;
    p->setup_mount = setup_mount;
    INIT_LIST_HEAD(&p->node);
    return p;
}

void vfs_init()
{
    rootfs = new_mnt(NULL, NULL);
    if ((rootfs->fs = new_fs("tmpfs", tmpfs_setup_mount)) == NULL) {
        uart_write_string("Fail on tmpfs initalization\n");
        return;
    }
    uart_write_string("vfs_init chk1\n");
    //mount on rootfs
    rootfs->fs->setup_mount(rootfs->fs, rootfs);
    uart_write_string("vfs_init chk2\n");
    //make root directory
    // tmpfs_mkdir(NULL, &rootfs->root, "");
    //register to file system list
    register_filesystem(rootfs->fs);
    uart_write_string("vfs_init chk3\n");

    //make initramfs under root directory
    vfs_mkdir("/initramfs");
    uart_write_string("vfs_init chk4\n");


    if (vfs_lookup("/initramfs", NULL)) {
        uart_write_string("initramfs does not exist.\n");
    }

    struct mount *initramfs = new_mnt(NULL, NULL);
    if (!initramfs) {
        uart_write_string("Fail on create struct mount *initramfs.\n");
    }
    if ((initramfs->fs = new_fs("initramfs", initramfs_setup_mount)) == NULL) {
        uart_write_string("Fail on initramfs initalization\n");
        return;
    }
    
    register_filesystem(initramfs->fs);
    vfs_mount("/initramfs", "initramfs");

    if (vfs_lookup("/initramfs/vfs1.img", NULL)) {
        uart_write_string("initramfs/vfs1.img does not exist.\n");
    }

    //Device files
    if (vfs_mkdir("/dev")) {
        uart_write_string("fail on make /dev\n");
        return;
    }

    if (vfs_lookup("/dev", NULL)) {
        uart_write_string("/dev does not exist.\n");
    }

    //uart
    vfs_mknod("/dev/uart", &uart_f_ops, CHARACTER_DEVICE);

    if (vfs_lookup("/dev/uart", NULL)) {
        uart_write_string("/dev/uart does not exist.\n");
    }

    //frame buffer
    vfs_mknod("/dev/framebuffer", &framebuffer_f_ops, BLOCK_DEVICE);

    if (vfs_lookup("/dev/framebuffer", NULL)) {
        uart_write_string("/dev/framebuffer does not exist.\n");
    }
}

int register_filesystem(struct filesystem* fs)
{
    // register the file system to the kernel.
    uart_write_string("register_filesystem chk1\n");
    if (fs_lookup(fs->name) == NULL) {
        uart_write_string("register_filesystem chk2\n");
        INIT_LIST_HEAD(&fs->node);
        list_add_tail(&fs->node, &fs_list);
        return 0;
    }
    uart_write_string("register_filesystem chk3\n");
    return 1;
}

struct file *_lookup_empty_file(task_t *t)
{
    for (int i = 0; i <= MAX_FD; i++) {
        if (t->open_files[i].vnode == NULL) {
            return &t->open_files[i];
        }
    }
    return NULL;
}

struct file *lookup_empty_file()
{
    task_t *current = get_current_thread();
    return _lookup_empty_file(current);
}



int vfs_create(const char *pathname, int flags, struct vnode **target)
{
    return _vfs_create(pathname, flags, target, REGULAR_FILE, NULL);
}

//if parent directory is not exists, create them.
//then, create file.
int _vfs_create(const char *pathname, int flags, struct vnode **target, enum FileType file_type, 
                int (*vnode_postprocess)(struct vnode* dir_node, struct vnode** target, const char* component_name)
            )
{
    //lookup file
    // should fail if file exist.
    struct vnode *existing = NULL;
    if (vfs_lookup(pathname, &existing) == 0) {
        //file exists
        return 3;
    }

    //if parent directory doesn't exists, create...
    char last_dir[MAX_PATHSIZE+1] = "";
    strcpy(last_dir, pathname);
    size_t len = strlen(last_dir);
    if (pathname[len-1] == '/') {
        //trying to open a directory
        uart_write_string("trying to open a directory!\n");
        return 2;
    }
    size_t dir_len;
    for (dir_len = len; dir_len >= 1 && last_dir[dir_len-1] != '/'; dir_len--)
        last_dir[dir_len-1] = '\0';
    struct vnode *parent_node;
    //create parent directory
    while (vfs_lookup(last_dir, &parent_node))
        vfs_mkdir(last_dir);
    
    //create a new file.
    vfs_lookup(last_dir, &parent_node);
    struct vnode *tmp = new_vnode(NULL, NULL, parent_node->f_ops, parent_node, file_type, NULL);
    if (tmp == NULL) {
        uart_write_string("_vfs_create: Fail on allocating tmp node for further process\n");
        return 1;
    }
    *target = tmp;
    int ret = (vnode_postprocess == NULL) ? parent_node->v_ops->create(parent_node, target, pathname + dir_len) : vnode_postprocess(parent_node, target, pathname + dir_len);
    if (ret != 0) {
        uart_write_string("_vfs_create: real create call fail\n");
        kfree(tmp);
        *target = NULL;
        return ret;
    }
    if (*target != tmp) {
        kfree(tmp);
        (*target)->parent = parent_node;
    }
    uart_write_string("create file: ");
    uart_write_string(pathname);
    uart_write_string(" ");
    uart_write_no_hex(*target);
    uart_write_string("\n");
    return 0;
}

/**
 * vfs_mknod(char *path, struct file_operations *dev_fops) - register a device file under path.
 * @param path: target device file path
 * @param dev_fops: device driver to override original file fops
 * Create a file node in vfs and override file node fops to dev_fops
 */
void vfs_mknod(char *path, struct file_operations *dev_fops, enum FileType file_type)
{
    struct vnode *target;
    // if (_vfs_create(path, FILE_READ | FILE_WRITE, &target, file_type, mknod_post)) {
    if (_vfs_create(path, FILE_READ | FILE_WRITE, &target, file_type, NULL)) {
        uart_write_string("vfs_mknod: _vfs_create fail!\n");
        return;
    }
    target->f_ops = dev_fops;
}

int vfs_open(const char *pathname, int flags, struct file **target)
{
    struct file *file;
    if ((file = lookup_empty_file()) == NULL) {
        uart_write_string("vfs_open: run out of empty file slot for current process.\n");
        return 1;
    }
    return _vfs_open(pathname, flags, target, file);
}

int _vfs_open(const char* pathname, int flags, struct file** target, struct file *file)
{
    int errno;
    // 2. Create a new file handle for this vnode if found.
    memset(file, 0, sizeof(struct file));

    // 1. Lookup pathname
    errno = vfs_lookup(pathname, &file->vnode);

    // 3. Create a new file if O_CREAT is specified in flags and vnode not found
    if (file->vnode == NULL && (flags & O_CREAT)) {
        //Newly created file is readable and writable.
        flags |= (FILE_READ | FILE_WRITE | FILE_APPEND);
        //Create a new file and its directory
        if ((errno = vfs_create(pathname, flags, &file->vnode)) != 0) {
            uart_write_string("_vfs_open: fail on creating a new file when O_CREAT is specified!\n");
            return errno;
        }
    }
    // lookup error code shows if file exist or not or other error occurs
    if (errno) {
        uart_write_string("File not found!\n");
        // 4. Return error code if fails
        return errno;
    }

    file->flags = flags;
    //residual actions
    *target = file;
    if (errno = file->vnode->f_ops->open(file->vnode, target) != 0) {
        //reset empty struct file
        uart_write_string("_vfs_open: fail on calling real open!\n");
        memset(file, 0, sizeof(struct file));
        *target = NULL; // avoid dangling pointer and potential security risks
        return errno;
    }
    return 0;
}

int vfs_close(struct file* file)
{
    int ret = file->f_ops->close(file);
    //file is in task_t. So, no need to use kfree
    // kfree(file);
    return ret;
}

int vfs_write(struct file* file, const void* buf, size_t len)
{
    if (!(file->flags & FILE_WRITE)) {
        //NOT IN WRITE MODE
        //no data is writen
        uart_write_string("vfs_write: permission deny.\n");
        return 0;
    }
    return file->f_ops->write(file, buf, len);
}

int vfs_read(struct file* file, void* buf, size_t len)
{
    if (!(file->flags & FILE_READ)) {
        //NOT IN READ MODE
        //no data is read
        uart_write_string("vfs_read: permission deny.\n");
        return 0;
    }
    return file->f_ops->read(file, buf, len);
}

long vfs_lseek64(struct file *file, long offset, int whence)
{
    return file->vnode->f_ops->lseek64(file, offset, whence);
}

//help vfs_mkdir make directory recursively.
int _vfs_mkdir(struct vnode *cur_dir, char **entries, int i)
{
    if (entries[i] == NULL)
        return 0;

    //if directory exists, do nothing
    //otherwise, create a subdir
    struct vnode *target_dir;
    int errno;
    if (cur_dir->v_ops->lookup(cur_dir, &target_dir, entries[i]) != 0) {
        struct vnode *tmp = new_vnode(NULL, cur_dir->v_ops, cur_dir->f_ops, cur_dir, DIRECTORY, NULL);
        if (tmp == NULL) {
            uart_write_string("_vfs_mkdir: Fail on allocating new vnode for new subdir!\n");
            return 1;
        }
        target_dir = tmp;
        if ((errno = cur_dir->v_ops->mkdir(cur_dir, &target_dir, entries[i]) != 0)) {
            uart_write_string("_vfs_mkdir: Fail on calling real mkdir!\n");
            kfree(tmp);
            target_dir = NULL;
            return errno;
        }
        if (target_dir != tmp)
            kfree(tmp);
    }
    //recursively call _vfs_mkdir to recursively create descendent directory
    return _vfs_mkdir(target_dir, entries, i+1);
}

int vfs_mkdir(const char* pathname)
{
    char dup[MAX_PATHSIZE+1];
    char *entry[MAX_PATHSIZE / 2 + 2];
    memset(entry, 0, sizeof(entry));
    strcpy(dup, pathname);
    int entry_cnt = vfs_parse_entry(dup, entry, MAX_PATHSIZE / 2 + 1);
    struct vnode *cur = (pathname[0] == '/') ? rootfs->root : get_current_thread()->cwd;

    return _vfs_mkdir(cur, entry, 0);
}

int vfs_mount(const char* target, const char* filesystem)
{
    struct filesystem *fs = fs_lookup(filesystem);
    if (fs == NULL) {
        uart_write_string("vfs_mount: mount fs not found!\n");
        return MOUNT_FS_NOT_FOUND;
    }
    struct vnode *mount_node;
    int errno;
    if ((errno = vfs_lookup(target, &mount_node)) != 0) {
        uart_write_string("vfs_mount: Something went wrong on vfs_lookup, errno: ");
        uart_write_no(errno);
        uart_write_string("\n");
        return 3;
    }
    if (mount_node->file_type != DIRECTORY) {
        uart_write_string("vfs_mount: mounted node is not a directory!\n");
        return 4;
    }

    mount_node->mount = new_mnt(NULL, fs);
    if (mount_node->mount == NULL) {
        uart_write_string("vfs_mount: Fail on allocating new mount!\n");
        return 2;
    }
    mount_node->mount->fs = fs;
    //create a root node for this mount
    struct vnode *mount_root = new_vnode(NULL, NULL, NULL, mount_node->parent, DIRECTORY, NULL);
    if (mount_root == NULL) {
        uart_write_string("vfs_mount: Fail on allocating a new vnode for mount root!\n");
        return 5;
    }

    //register it
    mount_node->mount->root = mount_root;
    // mount_node->mount->root = mount_node;
    
    return fs->setup_mount(fs, mount_node->mount);
}

int vfs_parse_entry(char *s, char *entry[], unsigned max_args)
{
    char *token = strtok(s, "/");
    int argc = 0;
    while (token != NULL && argc < max_args) {
        entry[argc++] = token;
        token = strtok(NULL, "/");
    }
    entry[argc] = NULL;
    return argc;
}

// Lookup vnode by pathname
// On success, return 0, else return error code > 0
int vfs_lookup(const char* pathname, struct vnode** target)
{
    char dup[MAX_PATHSIZE+1];
    char *entry[MAX_PATHSIZE / 2 + 2];
    memset(entry, 0, sizeof(entry));
    strcpy(dup, pathname);
    int entry_cnt = vfs_parse_entry(dup, entry, MAX_PATHSIZE / 2 + 1);
    //decide whether to accept relative path in pathname.
    //If so, should record CWD in task_t
    struct vnode *cur = (pathname[0] == '/') ? rootfs->root : get_current_thread()->cwd;
    int errno = 0;
    //vfs walk
    for (int i = 0; i < entry_cnt; i++) {
        if (strcmp(entry[i], ".") == 0) {
            //current directory
            continue;
        } else if (strcmp(entry[i], "..") == 0) {
            //TODO: root vnode of a mounted file system should set parent as mount point
            if (cur->parent) cur = cur->parent;
            continue;
        }
        //if current directory is a mount point
        if (cur->mount) {
            //switch to root inode of that file system
            cur = cur->mount->root;
        }
        /*
        registered function is responsible for
        1. lookup vnode named entry[i] in "cur" directory and set it in &cur.
        2. if not found, return errno > 0
        */
        if ((errno = cur->v_ops->lookup(cur, &cur, entry[i])) != 0) {
            uart_write_string("vfs_lookup: Fail on lookup ");
            uart_write_string(entry[i]);
            uart_write_string("\n");
            return errno;
        }
    }
    if (cur->mount) {
        cur = cur->mount->root;
    }
    if (target != NULL)
        *target = cur;
    return 0;
}

int vfs_chdir(const char *pathname)
{
    int errno;
    struct vnode *target_dir;
    if ((errno = vfs_lookup(pathname, &target_dir)) != 0) {
        uart_write_string("vfs_chdir: Fail on lookup ");
        uart_write_string(pathname);
        uart_write_string("\n");
        return errno;
    }
    if (target_dir->file_type != DIRECTORY) {
        uart_write_string("vfs_chdir: target_dir is not a directory!\n");
        return 1;
    }
    get_current_thread()->cwd = target_dir;
    return 0;
}

int example_mkdir(struct vnode* dir_node, struct vnode** target, const char* component_name)
{
    //make a new directory vnode under dir_node.
    //set up void *content as a list_t in subdir vnode.
    struct vnode *subdir = *target;
    INIT_LIST_HEAD((list_t *)(&subdir->internal));

    //create a dentry and add to parent directory
    if (dir_node) {
        //create a dentry
        struct dentry *dir_dent = new_dentry(component_name, DIRECTORY, subdir, NULL);

        //add to parent directory
        list_add_tail(&dir_dent->node, (list_t *)&(dir_node->internal));
    }
    return 0;
}

int example_lookup_d(struct vnode* dir_node, struct dentry** target, const char* component_name)
{
    if (dir_node->file_type != DIRECTORY) {
        //dir_node is not a directory
        return 2;
    }
    list_t *dir_head = (list_t *)(&dir_node->internal);
    struct dentry *entry, *safe;
    list_for_each_entry_safe(entry, safe, dir_head, node) {
        if (strcmp(entry->d_name, component_name) == 0) {
            *target = entry;
            return 0;
        }
    }
    //component_name is not found in dir_node
    return 1;
}

int example_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name)
{
    struct dentry *dent;
    int errno;
    if (errno = example_lookup_d(dir_node, &dent, component_name)) {
        return errno;
    }
    *target = (struct vnode *)(dent->content);
    return 0;
    // if (dir_node->file_type != DIRECTORY) {
    //     //dir_node is not a directory
    //     return 2;
    // }
    // list_t *dir_head = (list_t *)(&dir_node->internal);
    // struct dentry *entry, *safe;
    // list_for_each_entry_safe(entry, safe, dir_head, node) {
    //     if (strcmp(entry->d_name, component_name) == 0) {
    //         *target = (struct vnode *)(entry->content);
    //         return 0;
    //     }
    // }
    // //component_name is not found in dir_node
    // return 1;
}

void print_tree()
{
    uart_write_string("\n");
    _print_tree(rootfs->root, 0);
    uart_write_string("\n");
}

void _print_tree(struct vnode *dir_node, int depth)
{
    if (dir_node->mount)
        dir_node = dir_node->mount->root;
    struct dentry *entry, *safe;
    list_t *head = (list_t *)(&(dir_node->internal));
    list_for_each_entry_safe(entry, safe, head, node) {
        for (int i = 0; i < depth; i++) uart_write_string("  ");
        uart_write_string(entry->d_name);
        uart_write_string("\n");
        if (entry->file_type == DIRECTORY) {
            _print_tree(entry->content, depth+1);
        }
    }
}

void ls_dir(struct vnode *dir_node)
{
    uart_write_string("\n------------------------------------------\n");
    struct dentry *entry, *safe;
    list_t *head = (list_t *)(&(dir_node->internal));
    list_for_each_entry_safe(entry, safe, head, node) {
        if (entry->file_type == DIRECTORY) uart_write_string("d ");
        else uart_write_string("- ");

        uart_write_string(entry->d_name);
        uart_write_string("\t");
        if (entry->file_size)
            uart_write_no(*(entry->file_size));
        else
            uart_write_string("?");
        uart_write_string("\n");
    }
}

//file_link: NULL for create a new file
//           a node in the link if create a new hard link
int example_append2dir(struct vnode *dir_node, char *content, size_t *file_size, char *component_name, list_t *file_link, struct dentry **target)
{
    // ls_dir(dir_node);
    struct dentry *file_dent = new_dentry(component_name, REGULAR_FILE, content, file_size);
    if (file_dent == NULL)
        return 1;

    //In consider of hardlink, all dentries must be deleted once the file is removed.
    if (file_link) {
        list_add(&file_dent->file_link_node, file_link);
    }

    list_add_tail(&file_dent->node, (list_t *)(&dir_node->internal));
    *target = file_dent;
    // ls_dir(dir_node);
    return 0;
}

long default_lseek64(struct file *file, long offset, int whence, 
                    long (*get_file_size) (struct file *f),
                    long (*exceed_callback)(struct file *f, long new_offset), 
                    long (*below_callback)(struct file *f, long new_offset)
                )
{
    long errno;
    // adjust f_pos by whence first.
    switch (whence)
    {
    case SEEK_SET:
        file->f_pos = 0;
        break;
    case SEEK_END:
        file->f_pos = get_file_size(file);
        break;
    default: //other values or SEEK_CUR, leave f_pos on original position
        break;
    }
    // If the offset is beyond the end of the file and the file descriptor was opened in write mode,
    // the file is extended to accommodate the new offset. Normally, pad zero.
    long new_offset = (long)file->f_pos + offset;
    if (new_offset > get_file_size(file)) {
        return exceed_callback(file, new_offset);
        if (file->flags & FILE_WRITE) {
            if ((errno = exceed_callback(file, new_offset)) != 0) {
                return errno;
            }
            // //pad extend area with zero
            // memset(fnode->content_start + fnode->file_size, 0, file->f_pos + offset - fnode->file_size);
            file->f_pos += offset;
            return 0;
        } else {
            //If lseek() encounters an error during the seek operation, it returns -1 and sets the errno variable to indicate the specific error that occurred.
            return -1;
        }
    }
    // If the offset is negative and the file descriptor is opened in append mode, the offset will be treated as if it were zero. 
    // In all other cases, attempting to seek to a negative offset will result in an error.
    if (new_offset < 0) {
        if (file->flags & FILE_APPEND) {
            return below_callback(file, new_offset);
        } else {
            //report an error
            return -1;
        }
    }
    //new offset is within file range
    file->f_pos += offset;
    return 0;
}