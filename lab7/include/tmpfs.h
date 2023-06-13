#ifndef TMPFS_H
#define TMPFS_H

#include "vfs.h"
#define EOF 0
#define size_t int

void tmpfs_init();
int tmpfs_mount(struct filesystem* fs,struct mount* mount);
int tmpfs_write(struct file* file,const void* buf,size_t len);
int tmpfs_read(struct file* file,const void* buf,size_t len);
int tmpfs_open(struct vnode* file_node,struct file** target);
int tmpfs_close(struct file* file);
long tmpfs_lseek64(struct file* file,long offset,int whence);
int tmpfs_lookup(struct vnode* dir_node,struct vnode** target,const char* component_name);
int tmpfs_create(struct vnode* dir_node,struct vnode** target,const char* component_name);
int tmpfs_mkdir(struct vnode* dir_node,struct vnode** target,const char* component_name);

#endif
