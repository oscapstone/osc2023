#include "ramfs.h"
#include "heap.h"
#include "mem.h"
#include "uart.h"
#include "str.h"
#include <stdint.h>
#include <stddef.h>
#include "initrd.h"

extern struct mount* fsRootMount;
extern struct vnode* fsRoot;


/*
 * This function will build the hiechrachy of the cpio format
 */
int ramfs_initFsCpio(struct vnode* root){
	void *f = initrd_getLo();	// get the location of cpio
	struct vnode* target = NULL;
	char buf[16] = {0};
	uint64_t size;
	while(1){
		char* fname = initrd_getName(f);
		char* fdata = initrd_getData(f);
		int fmode =  initrd_getMode(f);
		//uart_puts(fname);
		// End
		if(strcmp(fname, "TRAILER!!!") == 0)
			break;
		struct vnode *dir_node = fsRoot;
		while(fname != NULL ){
			memset(buf, 0, 16);
			fname = getFileName(buf, fname);

			// The Directory must be the directory type
			((FsAttr*)(dir_node->internal))->type=DIRTYPE;
			uart_puts(buf);
			uart_puts("\n");
			if(*buf != 0){
				// Find if the dir is exist
				dir_node->v_ops->lookup(dir_node, &target, buf);
				// If not exist, just create it
				if(target == NULL)
					dir_node->v_ops->create(dir_node, &target, buf);

			}
			dir_node = target;
			target = NULL;
		}
		f = initrd_jumpNext(f);
	}
	return 0;
	
}

struct filesystem *getRamFs(void){
	struct filesystem* fs = malloc(sizeof(struct filesystem));
	fs->name = "Ramfs";
	fs->setup_mount = ramfs_init;
	return fs;
}


void data_init(FsAttr* fs){
	if(fs->data == NULL){
		fs->data = (void*)pmalloc(0);
	}
}

void ramfs_dump(struct vnode* cur, int depth){
	if(cur == NULL || cur->internal == NULL)
		return;
	FsAttr* fs = (FsAttr*)(cur->internal);
	for(int i = 0; i < depth; i++){
		uart_puts(" ");
	}
	if(fs->type == DIRTYPE){
		uart_puts("dir: ");
		uart_puts(fs->name);
		uart_puts("\n");
		struct vnode **f = (struct vnode**)(fs->dirs);
		for(unsigned i = 0; i < fs->size; i++){
			//uart_puth(f);
			//uart_puts(((FsAttr*)f[i]->internal)->name);
			ramfs_dump(f[i], depth + 1);
			//uart_puth(f[i]);
		}
	}
	else if(fs->type == NORMAL){
		uart_puts("normal: ");
		uart_puts(fs->name);
		uart_puts("\n");
	}
	else{
		uart_puts("Unknown file type\n");
	}
	return;
}


int ramfs_lookup(struct vnode *dir, struct vnode** target, const char* name){
	FsAttr *fs = (FsAttr*)dir->internal;
	if(fs->type != DIRTYPE){
		uart_puts("Lookup At Normal File\n");
		return 1;
	}
	struct vnode **c = (struct vnode**)fs->dirs;
	for(int i = 0; i < fs->size; i++){
		struct vnode* ch = c[i];
		FsAttr *cfs = (FsAttr*)ch->internal;
		/*
		uart_puts(" lookup: ");
		uart_puts(cfs->name);
		*/
		if(!strcmp(cfs->name, name)){
			*target = ch;
			/*
		uart_puts(" found: ");
		uart_puts(cfs->name);
		*/
		uart_puts("\n");
			return 0;
		}
	}
	*target = NULL;
	return 1;
}

int ramfs_create(struct vnode* dir, struct vnode** target, const char* name){
	FsAttr* fs = (FsAttr*)dir->internal;
	if(fs->type != DIRTYPE){
		uart_puts("U  should add file only in DIR\n");
		return 1;
	}

	if(fs->dirs == NULL)
		fs->dirs = (struct vnode**)malloc(sizeof(struct vnode*) * 16);
	struct vnode **c = (struct vnode**)fs->dirs;
	//*c = malloc(sizeof(void*) * 16);
	*target = (struct vnode*)malloc(sizeof(struct vnode));
	(*target)->mount = dir->mount;
	(*target)->parent = dir;
	(*target)->v_ops = dir->v_ops;
	(*target)->f_ops = dir->f_ops;
	(*target)->internal = malloc(sizeof(FsAttr));
	FsAttr* cfs = (FsAttr*)(*target)->internal;
	cfs->name = malloc(16);
	char* t = cfs->name;
	for(int i = 0; i < 16; i++){
		*t++ = *name++;
	}
	cfs->type = NORMAL;
	cfs->size = 0;
	cfs->data = NULL;
	// Update parent links
	c[(fs->size)++ ] = (*target);
	return 0;
}


int ramfs_init(struct filesystem* fs, struct mount* m){
	struct vnode* root = m->root;
	root->v_ops = (struct vnode_operations*) malloc(sizeof(struct vnode_operations));
	root->v_ops->lookup = ramfs_lookup;
	root->v_ops->create = ramfs_create;
	root->v_ops->mkdir = ramfs_mkdir;

	/*
	root->f_ops = (struct file_operations*) smalloc(64);
	root->f_ops->write = ramfs_write;
	root->f_ops->read = ramfs_read;
	root->f_ops->open = ramfs_open;
	root->f_ops->close = ramfs_close;
	*/

	char* name = NULL;

	// is not mount at the root of the whole FS
	if(root->internal != NULL)
		name = ((FsAttr*)root->internal)->name;
	else{
		name = malloc(sizeof(char) * 16);
		memset(name, 0, 16);
		*name = '/';
	}
	root->internal = (FsAttr*)malloc(sizeof(FsAttr));
	
	FsAttr* attr = root->internal;
	attr->name = name;
	attr->type = DIRTYPE;
	attr->size = 0;
	attr->dirs = (void*)smalloc(8 * sizeof(void*));
	return 0;
}

int ramfs_mkdir(struct vnode* dir, struct vnode** target, const char* name){
	ramfs_create(dir, target, name);
	FsAttr* attr = (FsAttr*)(*target)->internal;
	attr->type = DIRTYPE;
	return 0;
}
