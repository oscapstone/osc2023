#include "vfs.h"
#include "str.h"
#include "mem.h"
#include "uart.h"

// Note This should extern to the other file using this structrue
struct filesystem* FsArr[5] = {0};
struct vnode *fsRoot = NULL;	// This sould be set after tmpfs create
struct mount* *fsRootMount = NULL;
// Private counter, should not share to other files
static int FsCounter = 0;
static struct vnode *dir;	//FIXME: MT-unsafe
static struct vnode *root;

/*
 * Split the file name from /
 * similiar to strtok()
 */
char* getFileName(char* dest, const char* from){	
	int tmp = 0;
	char *f = from;
	for(int i = 0; i < 16; i ++){
		*dest = *f++;
		if((*dest) == 0){
			return NULL;
		}
		if((*dest) == '/'){
			*dest = 0;
			return f;
		}
		dest++;
	}
	return NULL;
}
	
// Register file system, using a global array to store it in the array
int register_filesystem(struct filesystem *fs){
	uart_puts("Register: ");
	uart_puts(fs->name);
	uart_puts("\n");
	FsArr[FsCounter++] = fs;
	if(FsCounter >= 5){
		FsCounter = 0;
	} return 0;
}


/*
 * TODO: Support recursive create
 */
int vfs_open(const char* pathName, int flags, struct file **target){
	*target = (struct file*)pmalloc(0); // Create teh File
	int ret;
	struct vnode *target_v = NULL;
	ret = vfs_lookup(pathName, &target_v);
	// If File not exist and the create not set
	if(target_v == NULL && flags | O_CREAT == 0){
		uart_puts("File Not exist\n");
		return 0;
	}
	if(target_v == NULL){
		dir->v_ops->create(dir, &target_v, pathName);
	}
	(*target)->vnode = target_v;
	(*target)->f_pos = 0;	// initial position
	(*target)->f_ops = dir->f_ops;	// Use the dirs fops
	(*target)->flags = flags;
	return 0;
}

int vfs_create(struct vnode* dir_node, struct vnode** target,
		const char * component_name){
	*target = (struct vnode*) pmalloc(0);
	(*target)->parent = dir_node;
	(*target)->mount = NULL;
	(*target)->v_ops = dir_node->v_ops;
	(*target)->f_ops = dir_node->f_ops;
	char * n = (*target)->name;
	for(int i = 0; i < 16; i ++){
		*n ++ = *component_name++;
		if(*n == 0)
			break;
	}
	(*target)->type = NORMAL;
	return 0;
}

int vfs_lookup(const char *name , struct vnode** target){
	char tmp[32];
	int i = 0;
	struct vnode* dir = *target;
	int ret;
	dir->v_ops->lookup(dir, &target, tmp);
	uart_puts("VFS lookup: \n");
	return 0;
}

int vfs_close(struct file *f){
	if(f != NULL){
		f->f_ops->close(f);
	}
	return 0;
}

int vfs_read(struct file *f, void* buf, size_t len){
	return f->f_ops->read(f, buf, len);
}

int vfs_write(struct file *f, const void* buf, size_t len){
	return f->f_ops->write(f, buf, len);
}

/* similiar to the create but the file type is dir
 * TODO: Recursive mkdir
 */
int vfs_mkdir(const char* path){
	// First, try to open the dir
	struct vnode* new_dir = NULL;
	struct file* file = NULL;
	vfs_open(path, O_CREAT, &file);
	if(file == NULL){
		uart_puts("Fail to MKDIR\n");
		return 1;
	}
	new_dir = file->vnode;
	new_dir->type = DIRTYPE;
	return 0;
}
	

