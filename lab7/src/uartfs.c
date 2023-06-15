#include "buddy.h"
#include "uartfs.h"
#include "vfs.h"
#include "mini_uart.h"

#define size_t int

struct file_operations* uartfs_f_ops;

void uartfs_init(struct mount* mount_node)
{
	uartfs_f_ops = d_alloc(sizeof(struct file_operations));
	uartfs_f_ops->write = uartfs_write;
	uartfs_f_ops->read = uartfs_read;
	uartfs_f_ops->open = uartfs_open;
	uartfs_f_ops->close = uartfs_close;

	struct filesystem *fs = d_alloc(sizeof(struct filesystem));
	fs->name = "uartfs";
	fs->setup_mount = uartfs_mount;
	register_filesystem(fs);
	fs->setup_mount(fs,mount_node);
	return;
}

int uartfs_mount(struct filesystem* fs,struct mount* mount)
{
	mount->fs= fs;
	struct vnode* tmp = d_alloc(sizeof(struct vnode));
	tmp->mount = mount;
	tmp->f_ops = uartfs_f_ops;
	tmp->node_type = "directory";
	mount->root = tmp;
	return 0;
}

int uartfs_write(struct file* file,const void* buf,size_t len)
{
	char* buffer = buf;
	for(int i=0;i<len;i++)
	{
		uart_send(*buffer++);
	}
	return len;
}

int uartfs_read(struct file* file,void* buf,size_t len)
{
	char* buffer = buf;
	for(int i=0;i<len;i++)
	{
		*buffer++ = uart_recv();
	}
	return len;
}

int uartfs_open(struct vnode* file_node,struct file** target)
{
	struct file* open_file = d_alloc(sizeof(struct file));
	open_file->vnode = file_node;
	open_file->f_ops = uartfs_f_ops;
	open_file->f_pos = 0;
	*target = open_file;
	return 0;
}

int uartfs_close(struct file* file)
{
	return 0;
}
