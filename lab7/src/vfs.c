#include "vfs.h"
#include "mem.h"
#include "str.h"
#include "uart.h"

// Note This should extern to the other file using this structrue
struct filesystem *FsArr[5] = {0};
struct vnode *fsRoot = NULL; // This sould be set after tmpfs create
struct mount **fsRootMount = NULL;
// Private counter, should not share to other files
static int FsCounter = 0;
static struct vnode *dir; // FIXME: MT-unsafe
static struct vnode *root;

/*
 * Split the file name from /
 * similiar to strtok()
 */
char *getFileName(char *dest, const char *from) {
  int tmp = 0;
  char *f = from;
  if (from == NULL) {
    *dest = 0;
    return NULL;
  }
  for (int i = 0; i < 16; i++) {
    *dest = *f++;
    if ((*dest) == 0) {
      return NULL;
    }
    if ((*dest) == '/') {
      *dest = 0;
      return f;
    }
    dest++;
  }
  return NULL;
}

// Register file system, using a global array to store it in the array
int register_filesystem(struct filesystem *fs) {
  uart_puts("Register: ");
  uart_puts(fs->name);
  uart_puts("\n");
  FsArr[FsCounter++] = fs;
  if (FsCounter >= 5) {
    FsCounter = 0;
  }
  return 0;
}

/***************************************************************
 * TODO: Support recursive create
 **************************************************************/
int vfs_open(const char *pathName, int flags, struct file **target, struct vnode* root) {
  *target = malloc(sizeof(struct file)); // Create the File
  struct vnode *dir = NULL;
  int ret;
  struct vnode *target_v = NULL;
  char *name = pathName;
  char *tmp = pathName;
  if(root == NULL)
	  dir = fsRoot;
  else
	  dir = root;

  // Get the basename of the pathName
  for (int i = 0; i < 16; i++) {
    if (*name == '/') {
      tmp = name + 1;
    }
    if (*name == 0)
      break;
    name++;
  }
  name = tmp;

  // Lookup if the file exist
  if(pathName[0] == '/')
  ret = vfs_lookup(pathName + 1, &target_v, root);
  else 
  ret = vfs_lookup(pathName, &target_v, root);
  // If File not exist and the create not set
  if (target_v == NULL && (flags | O_CREAT) == 0) {
    uart_puts("File Not exist\n");
    return 0;
  }
  if (target_v == NULL) {
    // Get the last dir to link to
    vfs_getLastDir(pathName, dir, &dir);
    dir->v_ops->create(dir, &target_v, name);
  }
  // let each fs implement their works
  target_v->f_ops->open(target_v, target);
  (*target)->flags = flags;
  return 0;
}

int vfs_create(struct vnode *dir_node, struct vnode **target,
               const char *component_name) {
  dir_node->v_ops->create(dir_node, target, component_name);
  return 0;
}

int vfs_lookup(const char *path, struct vnode **target, struct vnode* root) {
  struct vnode *dir = NULL;
  if(root == NULL){
	  dir = fsRoot;
 }
  else 
	  dir = root;
  char *name = path;
  if(*name == '/')
	  name = name + 1;
  char buf[16] = {0};
  while (1) {
    memset(buf, 0, 16);
    name = getFileName(buf, name);
    //uart_puts(buf);
    if (name == NULL && *buf == 0)
      break;
    else if (*buf != 0) {
      dir->v_ops->lookup(dir, target, buf);
    }
    if (name == NULL)
      break;
    if (*target == NULL)
      break;
    dir = *target;
    *target = NULL;
  }
  return 0;
}

int vfs_close(struct file *f) {
  if (f != NULL) {
    f->f_ops->close(f);
  }
  return 0;
}

int vfs_read(struct file *f, void *buf, size_t len) {
  return f->f_ops->read(f, buf, len);
}

int vfs_write(struct file *f, const void *buf, size_t len) {
  return f->f_ops->write(f, buf, len);
}

/* similiar to the create but the file type is dir
 * TODO: Recursive mkdir
 */
int vfs_mkdir(char *path, struct vnode* root) {
  struct vnode *dir = NULL;
  if(root == NULL)
	  dir = fsRoot;
  else
	  dir = root;
  struct vnode *target = NULL;
  char *name = path;
  if(*name == '/')
	  name += 1;
  char buf[16] = {0};
  while (1) {
    memset(buf, 0, 16);
    name = getFileName(buf, name);
    if (name == NULL && *buf == 0)
      break;
    if (name == NULL && *buf != 0) {
      dir->v_ops->lookup(dir, &target, buf);
      if (target != NULL) {
        uart_puts("MKDIR file already exist\n");
        return 1;
      }
      dir->v_ops->mkdir(dir, &target, buf);
    } else if (*buf != 0) {
      dir->v_ops->lookup(dir, &target, buf);
      if (target == NULL)
        dir->v_ops->create(dir, &target, buf);
    }
    if (target == NULL)
      break;
    dir = target;
    target = NULL;
  }
  return 0;
}

int vfs_getLastDir(char *path, struct vnode *dir_node, struct vnode **t) {
  struct vnode *dir = NULL;
  if (dir_node == NULL) {
    dir = fsRoot;
  } else
    dir = dir_node;
  if(*path == '/')
	  path += 1;

  char *name = path;
  char buf[16] = {0};
  struct vnode *target = NULL;
  while (1) {
    memset(buf, 0, 16);
    name = getFileName(buf, name);
    if (name == NULL)
      break;
    if (*buf != 0) {
      dir->v_ops->lookup(dir, &target, buf);
      if (target == NULL) {
        //	uart_puts("Cannot get the DIR\n");
        dir->v_ops->mkdir(dir, &target, buf);
      }
    }
    if (target == NULL)
      break;
    dir = target;
    target = NULL;
  }
  *t = dir;
  return 0;
}

//Rewrite the path to support .. and .
struct vnode* vfs_reWritePath(char* pathName, struct vnode *dir, char ** new_pathName){
	char* tmp = (char*) malloc(16);
	struct vnode* ret =  dir;
	memset(tmp, 0, 16);
	int p = 0;
	for(int i = 0; i < 16;){
		if(pathName[i] == '/'){
			strcpy(tmp + p, pathName + i);
			*new_pathName = tmp;
			return ret;
		}
		if(pathName[i] == '.' && pathName[i+1] == '.'){
			i += 3;
			ret = ret->parent;
		}
		if(pathName[i] == '.'){
			i += 2;
			ret = ret;
		}
		else{
			strcpy(tmp +p, pathName + i);
			*new_pathName = tmp;
			return ret;
		}
	}
	*new_pathName = tmp;
	return ret;
}

