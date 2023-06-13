#ifndef _VFS_H
#define _VFS_H
/*
  hard link is only available on file not directory!
*/
#include "utils.h"
#include "list.h"
#define MAX_FD 16
// For VFS the max pathanme length is 255.
#define MAX_PATHSIZE 255
#define LOOKUP_NOT_FOUND 1
#define MOUNT_FS_NOT_FOUND 1
#define O_CREAT                00000100
//beginning of the file
#define SEEK_SET 0
//current file offset
#define SEEK_CUR 1
//end of the file
#define SEEK_END 2
enum FileType {
  REGULAR_FILE,
  DIRECTORY,
  SYMBOLIC_LINK,
  CHARACTER_DEVICE,
  BLOCK_DEVICE,
  NAMED_PIPE,
  SOCKET
};

struct mount;

struct dentry {         
  char                     d_name[32];
  const enum FileType      file_type; //指定此項目的類型
  void                     *content; // a vnode
  const size_t             *file_size; //in consider of hard link
  //if file_type is DIRECTORY, connect directory next
  list_t                   node;
  //if file_type is REGULAR_FILE, connect dentries point to the same file
  list_t                   file_link_node;
};

extern struct dentry *new_dentry(
  char *d_name,
  enum FileType file_type,
  void *content,
  const size_t *file_size
);

/*
May refer to Linux design specified in man page: https://man7.org/linux/man-pages/man7/inode.7.html
*/
//索引節點 (inode) 則對應到目錄結構 (directory structure)，目錄中包含很多子項目，可能是檔案或資料夾，這些子項目稱為目錄項 (dentry)
struct vnode {
  struct mount* mount;
  struct vnode_operations* v_ops;
  struct file_operations* f_ops;
  struct vnode *parent;

  //is file_type necessary?
  enum FileType file_type;
  
  // The internal pointer is reserved for underlying file system to manipulate by themselves
  // VFS will provide an example directory structure, called dentry.
  // The behavior will treat internal as dentry_head, then store directory entry on the list.
  // My design:
  //    For tmpfs directory vnode, internal will be seen as follow.
  //    list_t dentry_head; where entries of files / directories under will be linked together.
  //    For tmpfs file vnode, internal will be seen as follow.
  //    struct tmpfs_file_node *fnode;
  //    For initramfs file node, the address of content will be store in internal.
  void* internal;
};
extern struct vnode *new_vnode(
    struct mount *mount, 
    struct vnode_operations* v_ops, 
    struct file_operations* f_ops, 
    struct vnode *parent, 
    enum FileType file_type, 
    void *internal
);
//example dentry functions
extern int example_mkdir(struct vnode* dir_node, struct vnode** target, const char* component_name);
extern int example_lookup_d(struct vnode* dir_node, struct dentry** target, const char* component_name);
extern int example_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name);
extern int example_append2dir(struct vnode *dir_node, char *content, size_t *file_size, char *component_name, list_t *file_link, struct dentry **target);

#define FILE_READ  (1 << 0)
#define FILE_WRITE (1 << 1)
#define FILE_APPEND (1 << 2)
// file handle
struct file {
  struct vnode* vnode; //point to file directory
  size_t f_pos;  // RW position of this file handle
  struct file_operations* f_ops;
  int flags; // open mode(rw)
};

struct mount {
  struct vnode* root; //point to root directory of the mount point
  struct filesystem* fs; //has file system name
};

extern struct mount *new_mnt(
  struct vnode *root,
  struct filesystem *fs
);

//physical file system list
extern list_t fs_list;
struct filesystem {
  const char* name;
  int (*setup_mount)(struct filesystem* fs, struct mount* mount);
  list_t node; //to connect physical file system list;
};

extern struct filesystem *new_fs(
  const char *name, 
  int (*setup_mount)(struct filesystem* fs, struct mount* mount)
);

struct file_operations {
  int (*write)(struct file* file, const void* buf, size_t len);
  int (*read)(struct file* file, void* buf, size_t len);
  int (*open)(struct vnode* file_node, struct file** target);
  int (*close)(struct file* file);
  long (*lseek64)(struct file* file, long offset, int whence);
};
//default behaviors for lseek64
extern long default_lseek64(struct file *file, long offset, int whence, 
                            long (*get_file_size) (struct file *f),
                            long (*exceed_callback)(struct file *f, long new_offset), 
                            long (*below_callback)(struct file *f, long new_offset)
                            );

struct vnode_operations {
  int (*lookup)(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
  int (*create)(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
  int (*mkdir)(struct vnode* dir_node, struct vnode** target,
              const char* component_name);
};

extern struct mount* rootfs;

//helper function
extern int vfs_parse_entry(char *s, char *entry[], unsigned max_args);

extern void vfs_init();
extern int vfs_chdir(const char *pathname);
extern int register_filesystem(struct filesystem* fs);
extern int _vfs_open(const char* pathname, int flags, struct file** target, struct file *file);
extern int vfs_open(const char* pathname, int flags, struct file** target);
extern int vfs_close(struct file* file);
extern int vfs_write(struct file* file, const void* buf, size_t len);
extern int vfs_read(struct file* file, void* buf, size_t len);
extern int vfs_mkdir(const char* pathname);
extern int vfs_mount(const char* target, const char* filesystem);
extern int vfs_lookup(const char* pathname, struct vnode** target);

// extern struct file *lookup_empty_file();
// extern struct file *_lookup_empty_file(task_t *t);
#endif