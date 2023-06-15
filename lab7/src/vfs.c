#include "vfs.h"
#include "thread.h"
#include "string.h"
#include "buddy.h"

#define null 0
#define size_t int
#define O_CREAT 0b100

extern struct thread *get_current();

struct mount* rootfs;
struct filesystem* fs_list = null;

void rootfs_init()
{
	rootfs = d_alloc(sizeof(struct mount));
	rootfs->root = null;
	return;
}

int register_filesystem(struct filesystem* fs)
{
	if(fs_list == null)
	{
		fs_list = fs;
		fs_list->next = null;
	}
	else
	{
		struct filesystem* tmp_list = fs_list;
		while(tmp_list->next != null)
		{
			tmp_list = tmp_list->next;
		}
		tmp_list->next = fs;
		fs->next = null;
	}
	return 0;
}

int vfs_lookup(const char* pathname,struct vnode** target)
{
/*
	uart_send_string(pathname);
	uart_send_string(" -> lookup pathname\n");
*/
	struct thread *thd = get_current();
	struct vnode *cur_dir = thd->cur_dir;

	int index = 0;
	char* dir_record = d_alloc(16);	//store component_name
	for(int i=0;i<16;i++)
	{
		dir_record[i] = 0;
	}

	for(int i=0;i<255;i++)			//max pathname is 255
	{
		if(pathname[i] == '/')
		{
			if(i == 0)
			{
				cur_dir = rootfs->root;		//pathname must start with /
			}
			else
			{
				if(strcmp(cur_dir->node_type,"directory") != 0)		//can't find go on
				{
					uart_send_string("lookup : this is not a directory!!\n");
					return -1;
				}
				struct vnode* next_dir;
				int op_status = cur_dir->v_ops->lookup(cur_dir,&next_dir,dir_record);	//search next_dir
				if(op_status < 0)
				{
					uart_send_string("lookup : lookup dir fail!!\n");
					return op_status;
				}
				if(next_dir->mount != null && next_dir->mount->root != null)	//if vnode have mount
				{
					next_dir = next_dir->mount->root;	//goto mount_node's root
				}
				cur_dir = next_dir;

				//reset dir_record
				index = 0;
				for(int i=0;i<16;i++)
				{
					dir_record[i] = 0;
				}
			}
		}
		else if(pathname[i] == 0)	//parse pathname end
		{
			if(index == 0)			//this is the last directory
			{
				*target = cur_dir;
				return 0;
			}
			else
			{
				if(strcmp(cur_dir->node_type,"directory") != 0)
				{
					uart_send_string("lookup : this is not a directory!!\n");
					return -1;
				}
				int op_status = cur_dir->v_ops->lookup(cur_dir,target,dir_record);
				if(op_status < 0)
				{
					uart_send_string("lookup : lookup dir fail!!\n");
					return op_status;
				}

				if(strcmp((*target)->node_type,"directory") == 0 && (*target)->mount != null && (*target)->mount->root != null)
				{
					*target = (*target)->mount->root;	//goto mount_node's root
				}
				return 0;
			}
		}
		else
		{
			dir_record[index] = pathname[i];
			index++;
		}
	}
	uart_send_string("lookup op fail!!\n");
	return -1;	//should not be here
}

int vfs_open(const char* pathname,int flags,struct file** target)
{
/*
	uart_send_string(pathname);
	uart_send_string(" -> pathname\n");
*/
	struct vnode* file_node;
	int op_status = vfs_lookup(pathname,&file_node);
	if(op_status < 0)	//vnode not found
	{
		if(flags && O_CREAT)	//new file need to creat
		{
			int last_dir_pos = 0;
			for(int i=0;i<40 && pathname[i] != 0;i++)
			{
				if(pathname[i] == '/')
				{
					last_dir_pos = i;
				}
			}

			char* file_path = d_alloc(255);
			for(int i=0;i<255;i++)
			{
				file_path[i] = 0;
			}
/*
			uart_int(last_dir_pos);
			uart_send_string(" -> last_dir_pos\n");
*/
			if(last_dir_pos > 0)
			{
				for(int i=0;i<last_dir_pos;i++)	//set pre_file_path
				{
					file_path[i] = pathname[i];
				}
			}
			else
			{
				file_path[0] = '/';
			}
/*
			uart_send_string(file_path);
			uart_send_string(" -> file path\n");
*/
			struct vnode* parent;
			op_status = vfs_lookup(file_path,&parent);	//pre_file_path must exist
			if(op_status < 0)
			{
				uart_send_string("open : lookup pre_file_path error\n");
				return op_status;
			}

			//clean file_path
			for(int i=0;i<255;i++)
			{
				file_path[i] = 0;
			}
			for(int i=last_dir_pos + 1;pathname[i] != 0 && i<255;i++)
			{
				file_path[i - last_dir_pos - 1] = pathname[i];	//set new file name
			}

			op_status = parent->v_ops->create(parent,&file_node,file_path);
			if(op_status < 0)
			{
				uart_send_string("open : create fail!!\n");
				return op_status;
			}
		}	
		else
		{
			uart_send_string("open : not found file & noneed to create!!\n");
			return op_status;
		}
	}
	int ret = file_node->f_ops->open(file_node,target);
	(*target)->flags = flags;
	return ret;
}

int vfs_close(struct file* file)
{
	if(file == null)
	{
		uart_send_string("invalid file to close!!\n");
		return -1;
	}
	return file->f_ops->close(file);
}

int vfs_write(struct file* file,const void* buf,size_t len)
{
	if(file == null)
	{
		uart_send_string("invalid file to write!!\n");
		return -1;
	}
	return file->f_ops->write(file,buf,len);
}

int vfs_read(struct file* file,void* buf,size_t len)
{
	if(file == null)
	{
		uart_send_string("invalid file to read!!\n");
		return -1;
	}
	file->f_pos = 0;	//start read from head
	return file->f_ops->read(file,buf,len);
}

int vfs_mkdir(const char* pathname)
{
/*
	uart_send_string(pathname);
	uart_send_string(" -> mkdir pathname\n");
*/
	char* file_path = d_alloc(255);
	for(int i=0;i<255;i++)
	{
		file_path[i] = 0;
	}

	char* dir_name = d_alloc(16);
	for(int i=0;i<16;i++)
	{
		dir_name[i] = 0;
	}

	int last_dir_pos = 0;
	for(int i=0;i<40 && pathname[i] != 0;i++)
	{
		if(pathname[i] == 0)
		{
			break;
		}
		if(pathname[i] == '/')
		{
			last_dir_pos = i;
		}
	}

	if(last_dir_pos > 0)
	{
		for(int i=0;i<last_dir_pos;i++)	//set pre_file_path
		{
			file_path[i] = pathname[i];
		}
	}
	else
	{
		file_path[0] = '/';
	}
/*
	uart_send_string(file_path);
	uart_send_string(" -> mkdir file_path\n");
*/
	for(int i=last_dir_pos + 1;i<255;i++)
	{
		if(pathname[i] == 0)
		{
			break;
		}
		dir_name[i - last_dir_pos - 1] = pathname[i];
	}
/*
	uart_send_string(dir_name);
	uart_send_string(" -> mkdir dir_name\n");
*/
	struct vnode* pre_path;
	struct vnode** target;

	vfs_lookup(file_path,&pre_path);
	int op_status = pre_path->v_ops->mkdir(pre_path,target,dir_name);
	if(op_status == 0)
	{
		(*target)->mount = null;
		return op_status;
	}
	uart_send_string("mkdir op fail!!\n");
	return op_status;
}

int vfs_mount(const char* target,const char* filesystem)
{
	struct vnode* mount_node;
	int op_status = vfs_lookup(target,&mount_node);
	if(op_status < 0)
	{
		uart_send_string("mount : lookup mount_node fail!!\n");
		return op_status;
	}
	if(mount_node->mount != null)
	{
		uart_send_string("this mount node had mount on other filesystem!!\n");
		return -1;
	}
	
	struct filesystem* fs = null;
	struct filesystem* tmp_list = fs_list;
	while(tmp_list != null)
	{
		if(strcmp(tmp_list->name,filesystem) == 0)
		{
			fs = tmp_list;
		}
		tmp_list = tmp_list->next;
	}
	if(fs == null)
	{
		return -1;
	}

	mount_node->mount = d_alloc(sizeof(struct mount));
	mount_node->mount->root = d_alloc(sizeof(struct vnode));
	mount_node->mount->root->mount = null;
	mount_node->mount->root->parent = mount_node->parent;
	return fs->setup_mount(fs,mount_node->mount);
}
