#include "tmpfs.h"
#include "vfs.h"
#include "buddy.h"

#define null 0

struct vnode_operations* tmpfs_v_ops;
struct file_operations* tmpfs_f_ops;

extern struct mount* rootfs;

void tmpfs_init()
{
	tmpfs_v_ops = d_alloc(sizeof(struct vnode_operations));
	tmpfs_f_ops = d_alloc(sizeof(struct file_operations));

	tmpfs_v_ops->lookup = tmpfs_lookup;
	tmpfs_v_ops->create = tmpfs_create;
	tmpfs_v_ops->mkdir = tmpfs_mkdir;

	tmpfs_f_ops->write = tmpfs_write;
	tmpfs_f_ops->read = tmpfs_read;
	tmpfs_f_ops->open = tmpfs_open;
	tmpfs_f_ops->close = tmpfs_close;
	tmpfs_f_ops->lseek64 = tmpfs_lseek64;

	struct filesystem *fs = d_alloc(sizeof(struct filesystem));
	fs->name = "tmpfs";
	fs->setup_mount = tmpfs_mount;
	register_filesystem(fs);
	fs->setup_mount(fs,rootfs);
	struct vnode* tmp;
	tmpfs_mkdir(rootfs->root,&tmp,"nothing1");	//bad method -> to solve child overwrite
	tmpfs_mkdir(rootfs->root,&tmp,"nothing2");
	tmpfs_mkdir(rootfs->root,&tmp,"initramfs");
	tmpfs_mkdir(rootfs->root,&tmp,"dev");
	vfs_mkdir("/dev/uart");
}

int tmpfs_mount(struct filesystem* fs,struct mount* mount)
{
	mount->fs = fs;
	struct vnode* tmp = d_alloc(sizeof(struct vnode));
	tmp->mount = mount;
	tmp->v_ops = tmpfs_v_ops;
	tmp->f_ops = tmpfs_f_ops;
	tmp->node_type = "directory";
	tmp->file_size = 0;
	//tmp->child_num = 0;

	if(mount->root != null)	//not mount on whole fs
	{
		tmp->parent = mount->root->parent;
		//tmp->child = rootfs->root->child;			//should set this , but it will have other bug
		//tmp->child_num = rootfs->root->child_num; //should set this , but it will have other bug
	}

	mount->root = tmp;
	return 0;
}

int tmpfs_write(struct file* file,const void* buf,size_t len)
{
	char* internal = file->vnode->internal;
	char* buffer = buf;
	for(int i=0;i<len;i++)
	{
		internal[file->f_pos] = buffer[i];
		file->f_pos++;
	}
	return len;
}

int tmpfs_read(struct file* file,const void* buf,size_t len)
{
	char* internal = file->vnode->internal;
	char* buffer = buf;

	if(strcmp(file->vnode->name,"vfs1.img") == 0)	//bad method
	{
		for(int i=0;i<len;i++)
		{
			buffer[i] = internal[file->f_pos];
			file->f_pos++;
		}
		return len;
	}

	for(int i=0;i<len;i++)
	{
		buffer[i] = internal[file->f_pos];
		file->f_pos++;
		if(buffer[i] == EOF)
		{
			return i;
		}
	}
	return len;
}

int tmpfs_open(struct vnode* file_node,struct file** target)
{
	struct file* open_file = d_alloc(sizeof(struct file));
	open_file->vnode = file_node;
	open_file->f_ops = tmpfs_f_ops;
	open_file->f_pos = 0;
	*target = open_file;
	return 0;
}

int tmpfs_close(struct file* file)
{
	if(file->f_pos > file->vnode->file_size)	//update if write file
	{
		file->vnode->file_size = file->f_pos;
	}
	return 0;
}

long tmpfs_lseek64(struct file* file,long offset,int whence)
{
	//soon coming   maybe
}

int tmpfs_lookup(struct vnode* dir_node,struct vnode** target,const char* component_name)
{
	if(strcmp(component_name,".") == 0)
	{
		*target = dir_node;
		return 0;
	}

	if(strcmp(component_name,"..") == 0)
	{
		*target = dir_node->parent;
		return 0;
	}
	struct vnode **childs = dir_node->child;

//	uart_send_string("*****lookup_list*****\n");

	for(int i=0;i<dir_node->child_num;i++)
	{
/*
		uart_send_string(childs[i]->name);
		uart_send_string("\n");
*/
		struct vnode* child = childs[i];
		if(strcmp(child->name,component_name) == 0)	//find directory
		{
			*target = child;
			return 0;
		}
	}

	*target = null;
	return -1;
}

int tmpfs_create(struct vnode* dir_node,struct vnode** target,const char* component_name)
{
	struct vnode* new_file;
	tmpfs_lookup(dir_node,&new_file,component_name);

	if(new_file != null)	//had create this file
	{
		*target = null;
		return -1;
	}
	new_file = d_alloc(sizeof(struct vnode));
	new_file->mount = null;
	new_file->v_ops = tmpfs_v_ops;
	new_file->f_ops = tmpfs_f_ops;
	char* tmp_internal = d_alloc(0x100);
	for(int i=0;i<0x100;i++)
	{
		*tmp_internal++ = 0;
	}
	new_file->internal = tmp_internal;
	new_file->file_size = 0;
	for(int i=0;i<16;i++)	//set name
	{
		new_file->name[i] = component_name[i];
		if(component_name[i] == 0)
		{
			break;
		}
	}
	new_file->node_type = "file";
	new_file->parent = dir_node;
	new_file->child = (struct vnode**)d_alloc(sizeof(struct vnode*) * 20);
	new_file->child_num = 0;
	*target = new_file;

	dir_node->child[dir_node->child_num++] = *target;
/*
	uart_int(dir_node->child_num);
	uart_send_string(" -> child_num\n");
*/
	return 0;
}

int tmpfs_mkdir(struct vnode* dir_node,struct vnode** target,const char* component_name)
{
	tmpfs_create(dir_node,target,component_name);
	(*target)->node_type = "directory";	//since tmpfs_create's default is file
	return 0;
}
